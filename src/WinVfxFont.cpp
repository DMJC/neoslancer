#include "neoslancer/WinVfxFont.h"

#include <cstring>

namespace neoslancer {

namespace {

uint32_t readU32LE(const std::vector<uint8_t>& data, size_t offset) {
    return static_cast<uint32_t>(data[offset]) | (static_cast<uint32_t>(data[offset + 1]) << 8) |
           (static_cast<uint32_t>(data[offset + 2]) << 16) | (static_cast<uint32_t>(data[offset + 3]) << 24);
}

bool inBounds(const std::vector<uint8_t>& data, size_t offset, size_t length) {
    return offset <= data.size() && length <= data.size() - offset;
}

} // namespace

bool parseWinVfxFont(const std::vector<uint8_t>& data, WinVfxFont& out) {
    if (!inBounds(data, 0, 0x10)) {
        return false;
    }

    const uint32_t charCount = readU32LE(data, 0x04);
    const uint32_t height = readU32LE(data, 0x08);
    if (height == 0 || charCount == 0 || charCount > 256) {
        return false;
    }
    if (!inBounds(data, 0x10, static_cast<size_t>(charCount) * 4)) {
        return false;
    }

    out = WinVfxFont{};
    out.height = height;

    for (uint32_t c = 0; c < charCount; ++c) {
        const uint32_t glyphOffset = readU32LE(data, 0x10 + c * 4);
        if (glyphOffset == 0) {
            continue;
        }
        if (!inBounds(data, glyphOffset, 4)) {
            continue; // malformed entry - skip rather than fail the whole font
        }
        const uint32_t width = readU32LE(data, glyphOffset);
        const size_t pixelBytes = static_cast<size_t>(width) * height;
        if (width == 0 || !inBounds(data, glyphOffset + 4, pixelBytes)) {
            continue;
        }

        WinVfxGlyph& glyph = out.glyphs[c];
        glyph.width = width;
        glyph.pixels.assign(data.begin() + static_cast<long>(glyphOffset + 4),
                             data.begin() + static_cast<long>(glyphOffset + 4 + pixelBytes));
    }

    return true;
}

} // namespace neoslancer
