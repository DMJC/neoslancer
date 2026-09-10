#include "neoslancer/TgaImage.h"

#include <array>

namespace neoslancer {

namespace {

uint16_t readU16LE(const std::vector<uint8_t>& data, size_t offset) {
    return static_cast<uint16_t>(data[offset]) | (static_cast<uint16_t>(data[offset + 1]) << 8);
}

bool inBounds(const std::vector<uint8_t>& data, size_t offset, size_t length) {
    return offset <= data.size() && length <= data.size() - offset;
}

using Rgba = std::array<uint8_t, 4>;

// Decodes one N-bit-per-channel color-map/pixel entry (15/16/24/32 bits,
// stored BGR(A) per the TGA spec and confirmed directly against
// SR_TGA_allocate_palette's own read loop - see TgaImage.h) into RGBA8.
Rgba decodeEntry(const uint8_t* bytes, int depth) {
    switch (depth) {
        case 15:
        case 16: {
            const uint16_t v = static_cast<uint16_t>(bytes[0]) | (static_cast<uint16_t>(bytes[1]) << 8);
            const uint8_t b = static_cast<uint8_t>((v & 0x1F) << 3);
            const uint8_t g = static_cast<uint8_t>(((v >> 5) & 0x1F) << 3);
            const uint8_t r = static_cast<uint8_t>(((v >> 10) & 0x1F) << 3);
            return {r, g, b, 255}; // bit 15 is a TGA "attribute" bit, not reliably alpha - ignored
        }
        case 24:
            return {bytes[2], bytes[1], bytes[0], 255};
        case 32:
            return {bytes[2], bytes[1], bytes[0], bytes[3]};
        default:
            return {255, 0, 255, 255}; // unsupported depth - obvious magenta rather than silently wrong
    }
}

// Reads the color map (if any) starting at `cursor`, advancing it past
// the map on success. Shared by parseTga (which then goes on to decode
// pixel data too) and parseTgaPalette (which stops here).
bool decodeColorMap(const std::vector<uint8_t>& data, size_t& cursor, uint8_t colorMapType, uint16_t colorMapLength,
                     uint8_t colorMapDepth, std::vector<Rgba>& colorMap) {
    if (colorMapType != 1 || colorMapLength == 0) {
        return true; // no color map - not an error, just nothing to do
    }
    const size_t bytesPerEntry = (static_cast<size_t>(colorMapDepth) + 7) / 8;
    const size_t colorMapBytes = static_cast<size_t>(colorMapLength) * bytesPerEntry;
    if (!inBounds(data, cursor, colorMapBytes)) {
        return false;
    }
    colorMap.reserve(colorMapLength);
    for (uint16_t i = 0; i < colorMapLength; ++i) {
        colorMap.push_back(decodeEntry(&data[cursor + static_cast<size_t>(i) * bytesPerEntry], colorMapDepth));
    }
    cursor += colorMapBytes;
    return true;
}

} // namespace

bool parseTga(const std::vector<uint8_t>& data, TgaImage& out) {
    if (!inBounds(data, 0, 18)) {
        return false;
    }

    const uint8_t idLength = data[0];
    const uint8_t colorMapType = data[1];
    const uint8_t imageType = data[2];
    const uint16_t colorMapLength = readU16LE(data, 5);
    const uint8_t colorMapDepth = data[7];
    const int width = readU16LE(data, 12);
    const int height = readU16LE(data, 14);
    const uint8_t bpp = data[16];
    const uint8_t imageDescriptor = data[17];

    if (width <= 0 || height <= 0 || bpp == 0) {
        return false;
    }

    size_t cursor = 18u + idLength;

    std::vector<Rgba> colorMap;
    if (!decodeColorMap(data, cursor, colorMapType, colorMapLength, colorMapDepth, colorMap)) {
        return false;
    }

    const bool isColorMapped = imageType == 1 || imageType == 9;
    const bool isGrayscale = imageType == 3 || imageType == 11;
    const bool isTruecolor = imageType == 2 || imageType == 10;
    const bool isRle = imageType == 9 || imageType == 10 || imageType == 11;
    if (!isColorMapped && !isGrayscale && !isTruecolor) {
        return false; // imageType 0 (no image data) or an unrecognized/unsupported value
    }

    const size_t bytesPerPixel = (static_cast<size_t>(bpp) + 7) / 8;
    if (bytesPerPixel == 0 || bytesPerPixel > 4) {
        return false;
    }

    const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);
    std::vector<uint32_t> rawValues(pixelCount, 0);

    auto readPixelValue = [&](size_t offset) -> uint32_t {
        uint32_t v = 0;
        for (size_t b = 0; b < bytesPerPixel; ++b) {
            v |= static_cast<uint32_t>(data[offset + b]) << (8 * b);
        }
        return v;
    };

    if (isRle) {
        size_t i = cursor;
        size_t written = 0;
        while (written < pixelCount && i < data.size()) {
            const uint8_t control = data[i++];
            const size_t count = static_cast<size_t>(control & 0x7F) + 1;
            const bool repeated = (control & 0x80) != 0;
            if (repeated) {
                if (!inBounds(data, i, bytesPerPixel)) {
                    return false;
                }
                const uint32_t value = readPixelValue(i);
                i += bytesPerPixel;
                for (size_t k = 0; k < count && written < pixelCount; ++k) {
                    rawValues[written++] = value;
                }
            } else {
                for (size_t k = 0; k < count && written < pixelCount; ++k) {
                    if (!inBounds(data, i, bytesPerPixel)) {
                        return false;
                    }
                    rawValues[written++] = readPixelValue(i);
                    i += bytesPerPixel;
                }
            }
        }
        if (written < pixelCount) {
            return false;
        }
    } else {
        if (!inBounds(data, cursor, pixelCount * bytesPerPixel)) {
            return false;
        }
        for (size_t p = 0; p < pixelCount; ++p) {
            rawValues[p] = readPixelValue(cursor + p * bytesPerPixel);
        }
    }

    out.width = width;
    out.height = height;
    out.rgba.assign(pixelCount * 4, 0);

    // Bit 5 of the image descriptor: 0 = origin at bottom (rows stored
    // bottom-to-top, the TGA default), 1 = origin at top (already
    // top-to-bottom). Bit 4: 1 = origin at right (columns stored
    // right-to-left). Normalize both to a plain top-to-bottom,
    // left-to-right RGBA8 buffer so callers never need to think about
    // on-disk storage order.
    const bool topToBottom = (imageDescriptor & 0x20) != 0;
    const bool rightToLeft = (imageDescriptor & 0x10) != 0;

    for (int row = 0; row < height; ++row) {
        const int srcRow = topToBottom ? row : (height - 1 - row);
        for (int col = 0; col < width; ++col) {
            const int srcCol = rightToLeft ? (width - 1 - col) : col;
            const uint32_t raw = rawValues[static_cast<size_t>(srcRow) * width + static_cast<size_t>(srcCol)];

            Rgba color{};
            if (isColorMapped) {
                color = (raw < colorMap.size()) ? colorMap[raw] : Rgba{255, 0, 255, 255};
            } else if (isGrayscale) {
                const uint8_t gray = static_cast<uint8_t>(raw & 0xFF);
                color = {gray, gray, gray, 255};
            } else {
                uint8_t bytes[4] = {static_cast<uint8_t>(raw & 0xFF), static_cast<uint8_t>((raw >> 8) & 0xFF),
                                     static_cast<uint8_t>((raw >> 16) & 0xFF), static_cast<uint8_t>((raw >> 24) & 0xFF)};
                color = decodeEntry(bytes, static_cast<int>(bytesPerPixel * 8));
            }

            const size_t dst = (static_cast<size_t>(row) * width + static_cast<size_t>(col)) * 4;
            out.rgba[dst + 0] = color[0];
            out.rgba[dst + 1] = color[1];
            out.rgba[dst + 2] = color[2];
            out.rgba[dst + 3] = color[3];
        }
    }

    return true;
}

bool parseTgaPalette(const std::vector<uint8_t>& data, WinVfxPalette& out) {
    if (!inBounds(data, 0, 18)) {
        return false;
    }

    const uint8_t idLength = data[0];
    const uint8_t colorMapType = data[1];
    const uint16_t colorMapLength = readU16LE(data, 5);
    const uint8_t colorMapDepth = data[7];
    if (colorMapType != 1 || colorMapLength == 0) {
        return false;
    }

    size_t cursor = 18u + idLength;
    std::vector<Rgba> colorMap;
    if (!decodeColorMap(data, cursor, colorMapType, colorMapLength, colorMapDepth, colorMap)) {
        return false;
    }

    // The real engine reads a fixed 256-entry/768-byte buffer
    // regardless of the file's own colorMapLength (Pass 61) - pad with
    // black if shorter, ignore anything beyond 256 if longer.
    for (size_t i = 0; i < out.colors.size(); ++i) {
        if (i < colorMap.size()) {
            out.colors[i] = {colorMap[i][0], colorMap[i][1], colorMap[i][2]};
        } else {
            out.colors[i] = {0, 0, 0};
        }
    }
    return true;
}

} // namespace neoslancer
