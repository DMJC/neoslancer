#include "neoslancer/WinVfxSprite.h"

namespace neoslancer {

namespace {

uint32_t readU32LE(const std::vector<uint8_t>& data, size_t offset) {
    return static_cast<uint32_t>(data[offset]) | (static_cast<uint32_t>(data[offset + 1]) << 8) |
           (static_cast<uint32_t>(data[offset + 2]) << 16) | (static_cast<uint32_t>(data[offset + 3]) << 24);
}

int32_t readI32LE(const std::vector<uint8_t>& data, size_t offset) {
    return static_cast<int32_t>(readU32LE(data, offset));
}

bool inBounds(const std::vector<uint8_t>& data, size_t offset, size_t length) {
    return offset <= data.size() && length <= data.size() - offset;
}

// Decodes one shape's RLE pixel stream. Returns false (leaving shape
// pixels/opaque empty) on any out-of-bounds read rather than throwing -
// a handful of real .spr entries have bogus min/max values (this port
// has seen 2 of 31 in a real FRONTEND.SPR) and are skipped this way
// rather than treated as a hard parse failure for the whole file.
bool decodeShapeRle(const std::vector<uint8_t>& data, size_t rleStart, WinVfxShape& shape) {
    const size_t pixelCount = static_cast<size_t>(shape.width) * static_cast<size_t>(shape.height);
    shape.pixels.assign(pixelCount, 0);
    shape.opaque.assign(pixelCount, 0);

    size_t i = rleStart;
    for (int row = 0; row < shape.height; ++row) {
        int x = 0;
        while (true) {
            if (i >= data.size()) return false;
            const uint8_t control = data[i++];
            const bool literal = (control & 1) != 0;
            const int count = control >> 1;

            if (count == 0 && !literal) {
                break; // end of row
            }
            if (count == 0 && literal) {
                if (i >= data.size()) return false;
                x += data[i++];
                continue;
            }
            if (!literal) {
                // solid run: one color byte, repeated `count` times
                if (i >= data.size()) return false;
                const uint8_t color = data[i++];
                for (int k = 0; k < count; ++k) {
                    if (x >= 0 && x < shape.width) {
                        const size_t idx = static_cast<size_t>(row) * shape.width + static_cast<size_t>(x);
                        shape.pixels[idx] = color;
                        shape.opaque[idx] = 1;
                    }
                    ++x;
                }
            } else {
                // literal run: `count` literal bytes
                if (!inBounds(data, i, static_cast<size_t>(count))) return false;
                for (int k = 0; k < count; ++k) {
                    const uint8_t color = data[i++];
                    if (x >= 0 && x < shape.width) {
                        const size_t idx = static_cast<size_t>(row) * shape.width + static_cast<size_t>(x);
                        shape.pixels[idx] = color;
                        shape.opaque[idx] = 1;
                    }
                    ++x;
                }
            }
        }
    }
    return true;
}

} // namespace

bool parseWinVfxSprite(const std::vector<uint8_t>& data, WinVfxSprite& out) {
    if (!inBounds(data, 0, 8)) {
        return false;
    }

    const uint32_t shapeCount = readU32LE(data, 0x04);
    if (shapeCount == 0 || !inBounds(data, 0x08, static_cast<size_t>(shapeCount) * 8)) {
        return false;
    }

    out = WinVfxSprite{};
    out.shapes.reserve(shapeCount);

    for (uint32_t s = 0; s < shapeCount; ++s) {
        const size_t entryOffset = 0x08 + static_cast<size_t>(s) * 8;
        const uint32_t descOffset = readU32LE(data, entryOffset);

        WinVfxShape shape;
        if (!inBounds(data, descOffset, 0x18)) {
            out.shapes.push_back(std::move(shape)); // empty placeholder, keeps indices aligned
            continue;
        }

        shape.minX = readI32LE(data, descOffset + 0x08);
        shape.minY = readI32LE(data, descOffset + 0x0c);
        shape.maxX = readI32LE(data, descOffset + 0x10);
        shape.maxY = readI32LE(data, descOffset + 0x14);
        const int64_t width = static_cast<int64_t>(shape.maxX) - shape.minX + 1;
        const int64_t height = static_cast<int64_t>(shape.maxY) - shape.minY + 1;

        // Real files contain a handful of bogus entries (garbage min/max
        // producing enormous or negative dimensions) - reject rather than
        // try to allocate/decode them.
        if (width <= 0 || height <= 0 || width > 8192 || height > 8192) {
            out.shapes.push_back(std::move(shape));
            continue;
        }

        shape.width = static_cast<int>(width);
        shape.height = static_cast<int>(height);
        if (!decodeShapeRle(data, descOffset + 0x18, shape)) {
            shape.pixels.clear();
            shape.opaque.clear();
            shape.width = 0;
            shape.height = 0;
        }

        out.shapes.push_back(std::move(shape));
    }

    return true;
}

} // namespace neoslancer
