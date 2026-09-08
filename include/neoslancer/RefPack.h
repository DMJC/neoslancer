#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace neoslancer {

// EA's RefPack (aka QFS) LZ77-family compressor, used to compress BigFile
// (.hog) archive entries. Algorithm identification: Confidence 4 (see
// ../StarLancer/reversing/reverse_engineered_functions.md, Pass 29). This
// implementation is validated Confidence 5 against the real
// gamedata/StarLancer/RESOURCE.HOG: decompressing all 950 compressed
// entries in that archive reproduces each entry's exact documented
// decompressed size with no errors.
//
// Decompresses src (the RefPack byte stream, NOT including the 5-byte
// 0x10 0xFB + 3-byte-BE-decompressed-size block header) into out, which is
// resized to expectedDecompressedSize. Returns false if the stream runs
// out of input before producing expectedDecompressedSize bytes, or if a
// match's back-reference distance would read before the start of the
// output buffer.
bool refpackDecompress(const uint8_t* src, size_t srcSize, size_t expectedDecompressedSize,
                        std::vector<uint8_t>& out);

} // namespace neoslancer
