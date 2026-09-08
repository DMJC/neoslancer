// Standalone dev tool for inspecting/extracting BigFile (.hog) archives.
// Not part of the game itself - built as a separate executable with no
// SDL2/OpenGL dependency, useful for validating BigFile.cpp against real
// archives and for pulling assets out during development.
#include "neoslancer/BigFile.h"
#include "neoslancer/WinVfxFont.h"
#include "neoslancer/WinVfxSprite.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>

namespace {

void printUsage(const char* argv0) {
    std::fprintf(stderr,
                  "usage:\n"
                  "  %s <archive.hog> --list\n"
                  "  %s <archive.hog> --extract <name> <outpath>\n"
                  "  %s <archive.hog> --extract-raw <exact-toc-name> <outpath>\n"
                  "  %s <archive.hog> --decode-fnt <name> <out.pgm>\n"
                  "  %s <archive.hog> --decode-spr <name> <outdir>\n"
                  "\n"
                  "--extract applies the engine's HOG_BigRead name normalization\n"
                  "(strip directory, strip a 2-char 'ut...' extension) before\n"
                  "looking the name up, matching how the game itself requests\n"
                  "resources. --extract-raw matches the exact TOC entry name\n"
                  "verbatim instead - use it for the rare entry (e.g. the real\n"
                  "RESOURCE.HOG's \"brgoodluck.ut\") that keeps an extension\n"
                  "--extract's normalization would strip.\n"
                  "\n"
                  "--decode-fnt/--decode-spr parse the WinVFX .fnt/.spr formats\n"
                  "(see WinVfxFont.h/WinVfxSprite.h) and dump a contact-sheet PGM\n"
                  "(font) or one PPM per shape (sprite) for visual inspection -\n"
                  "no palette is applied (none is ported yet), so sprite images\n"
                  "are greyscale-by-index with transparent pixels shown magenta.\n",
                  argv0, argv0, argv0, argv0, argv0);
}

void writePGM(const std::string& path, int width, int height, const std::vector<uint8_t>& gray) {
    std::ofstream out(path, std::ios::binary);
    out << "P5\n" << width << " " << height << "\n255\n";
    out.write(reinterpret_cast<const char*>(gray.data()), static_cast<std::streamsize>(gray.size()));
}

void writePPM(const std::string& path, int width, int height, const std::vector<uint8_t>& rgb) {
    std::ofstream out(path, std::ios::binary);
    out << "P6\n" << width << " " << height << "\n255\n";
    out.write(reinterpret_cast<const char*>(rgb.data()), static_cast<std::streamsize>(rgb.size()));
}

int decodeFnt(neoslancer::BigFileArchive& archive, const std::string& name, const std::string& outPath) {
    const std::vector<uint8_t> data = archive.read(name);
    if (data.empty()) {
        std::fprintf(stderr, "failed to read '%s' from archive\n", name.c_str());
        return 1;
    }

    neoslancer::WinVfxFont font;
    if (!neoslancer::parseWinVfxFont(data, font)) {
        std::fprintf(stderr, "failed to parse '%s' as a WinVFX font\n", name.c_str());
        return 1;
    }

    int glyphCount = 0;
    for (const auto& g : font.glyphs) {
        if (g.width > 0) ++glyphCount;
    }
    std::printf("font '%s': height=%u glyphs=%d\n", name.c_str(), font.height, glyphCount);

    constexpr int kCols = 16;
    const int rows = (glyphCount + kCols - 1) / kCols;
    const int cellW = 16;
    const int cellH = static_cast<int>(font.height) + 2;
    const int sheetW = kCols * cellW;
    const int sheetH = std::max(rows, 1) * cellH;
    std::vector<uint8_t> sheet(static_cast<size_t>(sheetW) * sheetH, 0);

    int cell = 0;
    for (int c = 0; c < 256; ++c) {
        const auto& glyph = font.glyphs[static_cast<size_t>(c)];
        if (glyph.width == 0) continue;
        const int cellX = (cell % kCols) * cellW;
        const int cellY = (cell / kCols) * cellH;
        for (uint32_t y = 0; y < font.height; ++y) {
            for (uint32_t x = 0; x < glyph.width && static_cast<int>(x) < cellW; ++x) {
                sheet[static_cast<size_t>(cellY + static_cast<int>(y)) * sheetW + cellX + static_cast<int>(x)] =
                    glyph.pixels[static_cast<size_t>(y) * glyph.width + x];
            }
        }
        ++cell;
    }

    writePGM(outPath, sheetW, sheetH, sheet);
    std::printf("wrote %dx%d contact sheet to '%s'\n", sheetW, sheetH, outPath.c_str());
    return 0;
}

int decodeSpr(neoslancer::BigFileArchive& archive, const std::string& name, const std::string& outDir) {
    const std::vector<uint8_t> data = archive.read(name);
    if (data.empty()) {
        std::fprintf(stderr, "failed to read '%s' from archive\n", name.c_str());
        return 1;
    }

    neoslancer::WinVfxSprite sprite;
    if (!neoslancer::parseWinVfxSprite(data, sprite)) {
        std::fprintf(stderr, "failed to parse '%s' as a WinVFX sprite\n", name.c_str());
        return 1;
    }

    std::printf("sprite '%s': %zu shapes\n", name.c_str(), sprite.shapes.size());
    std::filesystem::create_directories(outDir);

    int written = 0;
    for (size_t i = 0; i < sprite.shapes.size(); ++i) {
        const auto& shape = sprite.shapes[i];
        if (shape.width == 0 || shape.height == 0) {
            std::printf("  shape %zu: unparseable, skipped\n", i);
            continue;
        }
        std::vector<uint8_t> rgb(static_cast<size_t>(shape.width) * shape.height * 3);
        for (int p = 0; p < shape.width * shape.height; ++p) {
            if (shape.opaque[static_cast<size_t>(p)]) {
                const uint8_t v = shape.pixels[static_cast<size_t>(p)];
                rgb[static_cast<size_t>(p) * 3 + 0] = v;
                rgb[static_cast<size_t>(p) * 3 + 1] = v;
                rgb[static_cast<size_t>(p) * 3 + 2] = v;
            } else {
                rgb[static_cast<size_t>(p) * 3 + 0] = 255;
                rgb[static_cast<size_t>(p) * 3 + 1] = 0;
                rgb[static_cast<size_t>(p) * 3 + 2] = 255;
            }
        }
        const std::string path = outDir + "/shape" + std::to_string(i) + ".ppm";
        writePPM(path, shape.width, shape.height, rgb);
        ++written;
    }
    std::printf("wrote %d shape image(s) to '%s'\n", written, outDir.c_str());
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }

    const std::string archivePath = argv[1];
    const std::string command = argv[2];

    neoslancer::BigFileArchive archive;
    if (!archive.open(archivePath)) {
        std::fprintf(stderr, "failed to open '%s' as a BigFile archive\n", archivePath.c_str());
        return 1;
    }

    std::printf("%s: %zu entries\n", archivePath.c_str(), archive.entries().size());

    if (command == "--list") {
        for (const auto& entry : archive.entries()) {
            std::printf("%10u  %10u  %s\n", entry.offset, entry.size, entry.name.c_str());
        }
        return 0;
    }

    if (command == "--extract" || command == "--extract-raw") {
        if (argc < 5) {
            printUsage(argv[0]);
            return 1;
        }
        const std::string name = argv[3];
        const std::string outPath = argv[4];

        std::vector<uint8_t> data;
        if (command == "--extract-raw") {
            const neoslancer::BigFileTocEntry* entry = archive.findEntry(name);
            if (!entry) {
                std::fprintf(stderr, "no exact TOC entry named '%s'\n", name.c_str());
                return 1;
            }
            data = archive.readEntry(*entry);
        } else {
            data = archive.read(name);
        }

        if (data.empty()) {
            std::fprintf(stderr, "failed to read '%s' from archive\n", name.c_str());
            return 1;
        }

        std::ofstream out(outPath, std::ios::binary);
        if (!out.is_open()) {
            std::fprintf(stderr, "failed to open '%s' for writing\n", outPath.c_str());
            return 1;
        }
        out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        std::printf("wrote %zu bytes to '%s'\n", data.size(), outPath.c_str());
        return 0;
    }

    if (command == "--decode-fnt") {
        if (argc < 5) {
            printUsage(argv[0]);
            return 1;
        }
        return decodeFnt(archive, argv[3], argv[4]);
    }

    if (command == "--decode-spr") {
        if (argc < 5) {
            printUsage(argv[0]);
            return 1;
        }
        return decodeSpr(archive, argv[3], argv[4]);
    }

    printUsage(argv[0]);
    return 1;
}
