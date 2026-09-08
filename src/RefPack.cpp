#include "neoslancer/RefPack.h"

namespace neoslancer {

bool refpackDecompress(const uint8_t* src, size_t srcSize, size_t expectedDecompressedSize,
                        std::vector<uint8_t>& out) {
    out.clear();
    out.reserve(expectedDecompressedSize);

    size_t i = 0;
    auto need = [&](size_t count) { return i + count <= srcSize; };

    while (i < srcSize && out.size() < expectedDecompressedSize) {
        const uint8_t b0 = src[i++];
        size_t numLiterals = 0;
        size_t length = 0;
        size_t distance = 0;

        if (b0 < 0x80) {
            // Short match: 1 tag byte + 1 byte.
            if (!need(1)) return false;
            const uint8_t b1 = src[i++];
            numLiterals = b0 & 0x03;
            length = ((b0 >> 2) & 0x07) + 3;
            distance = ((static_cast<size_t>(b0) & 0x60) << 3) + b1 + 1;
        } else if (b0 < 0xC0) {
            // Medium match: 1 tag byte + 2 bytes.
            if (!need(2)) return false;
            const uint8_t b1 = src[i++];
            const uint8_t b2 = src[i++];
            numLiterals = (b1 >> 6) & 0x03;
            length = (b0 & 0x3F) + 4;
            distance = ((static_cast<size_t>(b1) & 0x3F) << 8) + b2 + 1;
        } else if (b0 < 0xE0) {
            // Long match: 1 tag byte + 3 bytes.
            if (!need(3)) return false;
            const uint8_t b1 = src[i++];
            const uint8_t b2 = src[i++];
            const uint8_t b3 = src[i++];
            numLiterals = b0 & 0x03;
            length = ((static_cast<size_t>(b0) & 0x0C) << 6) + b3 + 5;
            distance = ((static_cast<size_t>(b0) & 0x10) << 12) + (static_cast<size_t>(b1) << 8) + b2 + 1;
        } else if (b0 < 0xFC) {
            // Literal-only run, no match.
            numLiterals = ((static_cast<size_t>(b0) & 0x1F) << 2) + 4;
            if (!need(numLiterals)) return false;
            out.insert(out.end(), src + i, src + i + numLiterals);
            i += numLiterals;
            continue;
        } else {
            // Terminal tag: final short literal run, then stop.
            numLiterals = b0 & 0x03;
            if (!need(numLiterals)) return false;
            out.insert(out.end(), src + i, src + i + numLiterals);
            i += numLiterals;
            break;
        }

        if (!need(numLiterals)) return false;
        out.insert(out.end(), src + i, src + i + numLiterals);
        i += numLiterals;

        if (distance > out.size()) return false;
        const size_t start = out.size() - distance;
        for (size_t k = 0; k < length; ++k) {
            out.push_back(out[start + k]);
        }
    }

    return out.size() == expectedDecompressedSize;
}

} // namespace neoslancer
