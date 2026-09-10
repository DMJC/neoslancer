// Standalone Linux viewer for StarLancer's `.tga` assets (see
// TgaImage.h) - splash screens, VR room backdrops, `.bik` transition
// poster frames, and the palette.tga/softpal.tga files that turned out
// to be the real master-palette source (../StarLancer/reversing docs
// Pass 61), among other uses. Opens each file given on the command
// line directly (loose `.tga` files ship RefPack-compressed under
// RESOURCE/, same 0x10 0xFB marker as `.spr`/`.fnt` - auto-detected and
// decompressed the same way sprviewer does; the raw cd1/cd2
// disc-extracted copies are plain, uncompressed standard TGA and need
// no such unwrapping) and displays it in a resizable SDL2/OpenGL
// window, fit to the window preserving aspect ratio.
#include "neoslancer/RefPack.h"
#include "neoslancer/TgaImage.h"
#include "neoslancer/Window.h"
#include "neoslancer/gfx/Font.h"
#include "neoslancer/gfx/UIRenderer.h"

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

// Loose `.tga` files under RESOURCE/ ship RefPack-compressed, same
// 0x10 0xFB + 3-byte-BE-decompressed-size marker BigFile archive
// entries and `.spr`/`.fnt` files use - auto-detected here regardless
// of which kind of file this is, matching sprviewer's approach.
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

GLuint uploadRgba(const neoslancer::TgaImage& image) {
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 image.rgba.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

std::vector<uint8_t> swapRedBlue(const std::vector<uint8_t>& rgba) {
    std::vector<uint8_t> swapped = rgba;
    for (size_t i = 0; i + 3 < swapped.size(); i += 4) {
        std::swap(swapped[i], swapped[i + 2]);
    }
    return swapped;
}

void printUsage(const char* argv0) {
    std::fprintf(stderr,
                  "usage: %s <file.tga> [more.tga ...]\n"
                  "\n"
                  "Controls: Left/Right (or A/D) - prev/next file (when more than one given)\n"
                  "          R                  - swap red/blue channels\n"
                  "          Esc or Q           - quit\n",
                  argv0);
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::vector<std::string> paths(argv + 1, argv + argc);
    std::vector<neoslancer::TgaImage> images(paths.size());
    std::vector<bool> loaded(paths.size(), false);
    for (size_t i = 0; i < paths.size(); ++i) {
        const std::vector<uint8_t> data = loadPossiblyCompressed(paths[i]);
        if (data.empty()) {
            std::fprintf(stderr, "tgaviewer: failed to read/decompress '%s'\n", paths[i].c_str());
            continue;
        }
        if (!neoslancer::parseTga(data, images[i])) {
            std::fprintf(stderr, "tgaviewer: '%s' doesn't look like a Targa file\n", paths[i].c_str());
            continue;
        }
        loaded[i] = true;
    }
    if (std::none_of(loaded.begin(), loaded.end(), [](bool b) { return b; })) {
        std::fprintf(stderr, "tgaviewer: no file could be loaded\n");
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::fprintf(stderr, "tgaviewer: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    neoslancer::WindowConfig windowConfig;
    windowConfig.title = "tgaviewer";
    windowConfig.width = 1024;
    windowConfig.height = 768;
    neoslancer::Window window;
    if (!window.create(windowConfig)) {
        SDL_Quit();
        return 1;
    }

    if (TTF_Init() != 0) {
        std::fprintf(stderr, "tgaviewer: TTF_Init failed: %s\n", TTF_GetError());
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

    // Textures are built lazily per file, once, and cached here - two
    // per loaded file (normal + red/blue-swapped) so toggling R doesn't
    // re-upload every frame.
    std::vector<GLuint> textures(paths.size(), 0);
    std::vector<GLuint> swappedTextures(paths.size(), 0);
    bool redBlueSwapped = false;

    size_t index = 0;
    while (index < loaded.size() && !loaded[index]) {
        ++index;
    }

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
                } else if ((key == SDLK_RIGHT || key == SDLK_d) && paths.size() > 1) {
                    do {
                        index = (index + 1) % paths.size();
                    } while (!loaded[index]);
                } else if ((key == SDLK_LEFT || key == SDLK_a) && paths.size() > 1) {
                    do {
                        index = (index + paths.size() - 1) % paths.size();
                    } while (!loaded[index]);
                } else if (key == SDLK_r) {
                    redBlueSwapped = !redBlueSwapped;
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);
        const float windowW = static_cast<float>(window.width());
        const float windowH = static_cast<float>(window.height());
        renderer.drawRect(0, 0, windowW, windowH, neoslancer::Color{0.08f, 0.08f, 0.10f, 1.0f});

        const neoslancer::TgaImage& image = images[index];
        constexpr float kHeaderHeight = 40.0f;
        constexpr float kFooterHeight = 32.0f;
        if (image.width > 0 && image.height > 0) {
            GLuint& texture = redBlueSwapped ? swappedTextures[index] : textures[index];
            if (texture == 0) {
                if (redBlueSwapped) {
                    neoslancer::TgaImage swapped = image;
                    swapped.rgba = swapRedBlue(image.rgba);
                    texture = uploadRgba(swapped);
                } else {
                    texture = uploadRgba(image);
                }
            }

            const float availW = std::max(windowW - 40.0f, 1.0f);
            const float availH = std::max(windowH - kHeaderHeight - kFooterHeight - 20.0f, 1.0f);
            float scale = std::min(availW / static_cast<float>(image.width), availH / static_cast<float>(image.height));
            scale = std::min(scale, 8.0f); // don't blow up tiny images absurdly large
            if (scale <= 0.0f) {
                scale = 1.0f;
            }
            const float drawW = static_cast<float>(image.width) * scale;
            const float drawH = static_cast<float>(image.height) * scale;
            const float drawX = (windowW - drawW) * 0.5f;
            const float drawY = kHeaderHeight + (availH - drawH) * 0.5f + 10.0f;
            renderer.drawTexture(texture, drawX, drawY, drawW, drawH);
        }

        if (font.isLoaded()) {
            char line[320];
            const std::string baseName = std::filesystem::path(paths[index]).filename().string();
            if (paths.size() > 1) {
                std::snprintf(line, sizeof(line), "%s  -  file %zu / %zu", baseName.c_str(), index + 1, paths.size());
            } else {
                std::snprintf(line, sizeof(line), "%s", baseName.c_str());
            }
            renderer.drawText(font, line, 12.0f, 8.0f, neoslancer::Color{0.9f, 0.95f, 1.0f, 1.0f});

            if (image.width > 0) {
                std::snprintf(line, sizeof(line), "%dx%d%s", image.width, image.height,
                              redBlueSwapped ? "   (R/B swapped)" : "");
            } else {
                std::snprintf(line, sizeof(line), "failed to load - see stderr");
            }
            int lineW = 0, lineH = 0;
            renderer.measureText(font, line, lineW, lineH);
            renderer.drawText(font, line, windowW - static_cast<float>(lineW) - 12.0f, 8.0f,
                              neoslancer::Color{0.7f, 0.75f, 0.8f, 1.0f});

            const std::string hint =
                paths.size() > 1 ? "Left/Right: prev/next   R: swap red/blue   Esc/Q: quit" : "R: swap red/blue   Esc/Q: quit";
            int hintW = 0, hintH = 0;
            renderer.measureText(font, hint, hintW, hintH);
            renderer.drawText(font, hint, (windowW - static_cast<float>(hintW)) * 0.5f, windowH - kFooterHeight + 6.0f,
                              neoslancer::Color{0.55f, 0.55f, 0.6f, 1.0f});
        }

        window.swap();
    }

    for (GLuint texture : textures) {
        if (texture != 0) {
            glDeleteTextures(1, &texture);
        }
    }
    for (GLuint texture : swappedTextures) {
        if (texture != 0) {
            glDeleteTextures(1, &texture);
        }
    }
    renderer.shutdown();
    font.close();
    TTF_Quit();
    window.destroy();
    SDL_Quit();
    return 0;
}
