// Standalone dev tool for inspecting/extracting BigFile (.hog) archives.
// Not part of the game itself - built as a separate executable with no
// SDL2/OpenGL dependency, useful for validating BigFile.cpp against real
// archives and for pulling assets out during development.
#include "neoslancer/BigFile.h"

#include <cstdio>
#include <fstream>

namespace {

void printUsage(const char* argv0) {
    std::fprintf(stderr,
                  "usage:\n"
                  "  %s <archive.hog> --list\n"
                  "  %s <archive.hog> --extract <name> <outpath>\n"
                  "  %s <archive.hog> --extract-raw <exact-toc-name> <outpath>\n"
                  "\n"
                  "--extract applies the engine's HOG_BigRead name normalization\n"
                  "(strip directory, strip a 2-char 'ut...' extension) before\n"
                  "looking the name up, matching how the game itself requests\n"
                  "resources. --extract-raw matches the exact TOC entry name\n"
                  "verbatim instead - use it for the rare entry (e.g. the real\n"
                  "RESOURCE.HOG's \"brgoodluck.ut\") that keeps an extension\n"
                  "--extract's normalization would strip.\n",
                  argv0, argv0, argv0);
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

    printUsage(argv[0]);
    return 1;
}
