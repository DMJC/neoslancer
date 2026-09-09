// Standalone Linux viewer for the WinVFX `.spr` sprite-sheet format (see
// WinVfxSprite.h). Opens a real, on-disk `.spr` file directly (loose
// `.spr` files ship RefPack-compressed - Pass 41 - so this auto-detects
// and decompresses the same 0x10 0xFB marker BigFile entries use) and
// steps through its shapes one at a time in a resizable SDL2/OpenGL
// window, using the same real GL texture rendering path
// (WinVfxRenderer::drawShapeScaled) the actual menu screens use.
//
// A `.ccb` master palette is optional (also auto-decompressed if
// RefPack-compressed) - without one, shapes render as a synthetic
// identity greyscale palette (index N -> RGB(N,N,N)) so opaque/
// transparent structure and RLE decode correctness are still visible
// even with no real color data at hand.
#include "neoslancer/RefPack.h"
#include "neoslancer/Window.h"
#include "neoslancer/WinVfxPalette.h"
#include "neoslancer/WinVfxSprite.h"
#include "neoslancer/gfx/Font.h"
#include "neoslancer/gfx/UIRenderer.h"
#include "neoslancer/gfx/WinVfxRenderer.h"

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

std::vector<uint8_t> readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {};
    }
    const std::streamsize size = file.tellg();
    if (size <= 0) {
        return {};
    }
    file.seekg(0);
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}

// Loose `.spr`/`.ccb` files on disk may or may not be RefPack-compressed
// (loose `.spr`s always are; loose `.ccb`s observed so far aren't) - this
// checks the same 2-byte 0x10 0xFB marker BigFile archive entries use
// regardless of which file type it's given, rather than assuming either
// way.
std::vector<uint8_t> loadPossiblyCompressed(const std::string& path) {
    std::vector<uint8_t> raw = readFile(path);
    if (raw.size() < 5 || raw[0] != 0x10 || raw[1] != 0xFB) {
        return raw;
    }
    const uint32_t decompressedSize =
        (static_cast<uint32_t>(raw[2]) << 16) | (static_cast<uint32_t>(raw[3]) << 8) | raw[4];
    std::vector<uint8_t> out;
    if (!neoslancer::refpackDecompress(raw.data() + 5, raw.size() - 5, decompressedSize, out)) {
        return {};
    }
    return out;
}

void printUsage(const char* argv0) {
    std::fprintf(stderr,
                  "usage: %s <file.spr> [palette.ccb]\n"
                  "\n"
                  "Controls: Left/Right (or A/D) - prev/next shape\n"
                  "          Home/End           - first/last shape\n"
                  "          R                  - swap red/blue channels\n"
                  "          Esc or Q           - quit\n",
                  argv0);
}

// R/B channel swap, e.g. to sanity-check a palette parsed with the wrong
// byte order (BGR vs RGB) against a known-good result.
neoslancer::WinVfxPalette swapRedBlue(const neoslancer::WinVfxPalette& source) {
    neoslancer::WinVfxPalette swapped = source;
    for (auto& color : swapped.colors) {
        std::swap(color[0], color[2]);
    }
    return swapped;
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    const std::string sprPath = argv[1];
    const std::string palettePath = argc >= 3 ? argv[2] : "";

    const std::vector<uint8_t> sprData = loadPossiblyCompressed(sprPath);
    if (sprData.empty()) {
        std::fprintf(stderr, "sprviewer: failed to read/decompress '%s'\n", sprPath.c_str());
        return 1;
    }

    neoslancer::WinVfxSprite sprite;
    if (!neoslancer::parseWinVfxSprite(sprData, sprite) || sprite.shapes.empty()) {
        std::fprintf(stderr, "sprviewer: '%s' doesn't look like a WinVFX .spr file\n", sprPath.c_str());
        return 1;
    }

    neoslancer::WinVfxPalette palette;
    bool paletteLoaded = false;
    if (!palettePath.empty()) {
        const std::vector<uint8_t> palData = loadPossiblyCompressed(palettePath);
        paletteLoaded = !palData.empty() && neoslancer::parseWinVfxPalette(palData, palette);
        if (!paletteLoaded) {
            std::fprintf(stderr, "sprviewer: warning: failed to load palette '%s', using greyscale\n",
                          palettePath.c_str());
        }
    }
    if (!paletteLoaded) {
        for (size_t i = 0; i < 256; ++i) {
            const uint8_t v = static_cast<uint8_t>(i);
            palette.colors[i] = {v, v, v};
        }
    }
    const neoslancer::WinVfxPalette swappedPalette = swapRedBlue(palette);
    bool redBlueSwapped = false;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::fprintf(stderr, "sprviewer: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    neoslancer::WindowConfig windowConfig;
    windowConfig.title = "sprviewer - " + std::filesystem::path(sprPath).filename().string();
    windowConfig.width = 1024;
    windowConfig.height = 768;
    neoslancer::Window window;
    if (!window.create(windowConfig)) {
        SDL_Quit();
        return 1;
    }

    if (TTF_Init() != 0) {
        std::fprintf(stderr, "sprviewer: TTF_Init failed: %s\n", TTF_GetError());
        window.destroy();
        SDL_Quit();
        return 1;
    }
    neoslancer::Font font;
    font.loadSystemFont(16); // non-fatal if it fails - overlay text just won't render

    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    neoslancer::UIRenderer renderer;
    if (!renderer.init()) {
        font.close();
        TTF_Quit();
        window.destroy();
        SDL_Quit();
        return 1;
    }
    renderer.resize(window.width(), window.height());

    neoslancer::WinVfxRenderer vfx;

    size_t index = 0;
    while (index < sprite.shapes.size() && sprite.shapes[index].width == 0) {
        ++index; // land on the first actually-decoded shape, if any
    }
    if (index >= sprite.shapes.size()) {
        index = 0;
    }

    const std::string baseName = std::filesystem::path(sprPath).filename().string();
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                window.onResize(event.window.data1, event.window.data2);
                renderer.resize(window.width(), window.height());
            } else if (event.type == SDL_KEYDOWN) {
                const SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE || key == SDLK_q) {
                    running = false;
                } else if (key == SDLK_RIGHT || key == SDLK_d) {
                    index = (index + 1) % sprite.shapes.size();
                } else if (key == SDLK_LEFT || key == SDLK_a) {
                    index = (index + sprite.shapes.size() - 1) % sprite.shapes.size();
                } else if (key == SDLK_HOME) {
                    index = 0;
                } else if (key == SDLK_END) {
                    index = sprite.shapes.size() - 1;
                } else if (key == SDLK_r) {
                    redBlueSwapped = !redBlueSwapped;
                    // The shape texture cache is keyed on (sprite, shape
                    // index) only, not the palette used to bake it - clear
                    // it so the next draw rebuilds with the other palette.
                    vfx.shutdown();
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);
        const float windowW = static_cast<float>(window.width());
        const float windowH = static_cast<float>(window.height());
        renderer.drawRect(0, 0, windowW, windowH, neoslancer::Color{0.08f, 0.08f, 0.10f, 1.0f});

        const auto& shape = sprite.shapes[index];
        constexpr float kHeaderHeight = 56.0f;
        constexpr float kFooterHeight = 32.0f;
        if (shape.width > 0 && shape.height > 0) {
            const float availW = std::max(windowW - 40.0f, 1.0f);
            const float availH = std::max(windowH - kHeaderHeight - kFooterHeight - 20.0f, 1.0f);
            float scale = std::min(availW / static_cast<float>(shape.width), availH / static_cast<float>(shape.height));
            scale = std::min(scale, 8.0f); // don't blow up tiny icons absurdly large
            if (scale <= 0.0f) {
                scale = 1.0f;
            }
            const float drawW = static_cast<float>(shape.width) * scale;
            const float drawH = static_cast<float>(shape.height) * scale;
            const float drawX = (windowW - drawW) * 0.5f;
            const float drawY = kHeaderHeight + (availH - drawH) * 0.5f + 10.0f;
            vfx.drawShapeScaled(renderer, sprite, index, redBlueSwapped ? swappedPalette : palette, drawX, drawY,
                                drawW, drawH);
        }

        if (font.isLoaded()) {
            char line[256];
            std::snprintf(line, sizeof(line), "%s  -  shape %zu / %zu", baseName.c_str(), index,
                          sprite.shapes.size() - 1);
            renderer.drawText(font, line, 12.0f, 8.0f, neoslancer::Color{0.9f, 0.95f, 1.0f, 1.0f});

            if (shape.width > 0 && shape.height > 0) {
                std::snprintf(line, sizeof(line), "%dx%d   bounds (%d,%d)-(%d,%d)%s%s", shape.width, shape.height,
                              shape.minX, shape.minY, shape.maxX, shape.maxY,
                              paletteLoaded ? "" : "   (no palette - greyscale by index)",
                              redBlueSwapped ? "   (R/B swapped)" : "");
            } else {
                std::snprintf(line, sizeof(line), "unparseable (bogus bounds) - see WinVfxSprite.h");
            }
            renderer.drawText(font, line, 12.0f, 30.0f, neoslancer::Color{0.7f, 0.75f, 0.8f, 1.0f});

            const std::string hint =
                "Left/Right: prev/next   Home/End: first/last   R: swap red/blue   Esc/Q: quit";
            int hintW = 0, hintH = 0;
            renderer.measureText(font, hint, hintW, hintH);
            renderer.drawText(font, hint, (windowW - static_cast<float>(hintW)) * 0.5f, windowH - kFooterHeight + 6.0f,
                              neoslancer::Color{0.55f, 0.55f, 0.6f, 1.0f});
        }

        window.swap();
    }

    vfx.shutdown();
    renderer.shutdown();
    font.close();
    TTF_Quit();
    window.destroy();
    SDL_Quit();
    return 0;
}
