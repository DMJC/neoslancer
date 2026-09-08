#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace neoslancer {

// One entry from a BigFile (.hog) archive's table of contents.
struct BigFileTocEntry {
    uint32_t offset; // absolute byte offset of the entry's data in the archive
    uint32_t size;   // byte size of the entry's data as stored (compressed or not)
    std::string name;
};

// Reader for the engine's ".hog"/BigFile archive format
// (../StarLancer/reversing/reverse_engineered_functions.md, Pass 28/29;
// C:\lancer\game\bigfile.cpp / hog_file.cpp in the original source tree).
//
// Container layout, Confidence 5 (byte-exact validated this session
// against the real gamedata/StarLancer/RESOURCE.HOG — all 967 TOC entries
// parsed cleanly, and max(offset+size) across every entry equals the
// archive's own total file size exactly):
//
//   offset 0x00: magic, 4 bytes big-endian == "BIGF" (0x42494746)
//   offset 0x04: total archive file size, 4 bytes big-endian
//   offset 0x08: TOC entry count, 4 bytes big-endian
//   offset 0x0c: TOC region size in bytes (header included), big-endian
//
// NOTE: this corrects the reversing docs' Pass 28 field order (which has
// entry count at 0x04 and TOC size at 0x08, with 0x0c unused) — that
// ordering does not match the real archive's bytes; the order above does,
// exactly. Worth fixing at the source (reversing/reverse_engineered_functions.md
// lines 3996-4001) in a future pass over there.
//
// The TOC region is `tocSizeBytes` bytes read from the START of the file
// (so it re-includes the 16-byte header), and entries run from
// tocBuffer+0x10 to tocBuffer+tocSizeBytes:
//
//   struct BigFileTocEntry {
//       uint32_t fileOffsetBE;
//       uint32_t fileSizeBE;
//       char     name[];  // NUL-terminated, case-insensitive compared
//   };
//
// A resource's data, at its TOC-listed offset, is either raw bytes (read
// directly) or a RefPack-compressed block: 2-byte magic (0x10 0xFB) + a
// 3-byte-big-endian decompressed size, followed by the compressed stream
// (see RefPack.h; the exact 5-byte header shape here is this session's
// own empirical finding against real compressed entries, not from the
// docs, which only said "a 5-byte header").
class BigFileArchive {
public:
    BigFileArchive() = default;
    ~BigFileArchive();

    BigFileArchive(const BigFileArchive&) = delete;
    BigFileArchive& operator=(const BigFileArchive&) = delete;

    bool open(const std::string& path);
    void close();
    bool isOpen() const { return m_stream.is_open(); }

    const std::vector<BigFileTocEntry>& entries() const { return m_entries; }
    const BigFileTocEntry* findEntry(const std::string& name) const;

    // Reads and, if necessary, decompresses a named resource. name is
    // matched case-insensitively against the TOC after stripping any
    // leading directory component and any 2-character extension starting
    // with "ut" (see HOG_BigRead's stripping rule, Pass 29). Falls back to
    // a plain loose file on disk (relative to the archive's own directory)
    // if the name isn't found in the TOC, matching HOG_file_read.
    // Returns an empty vector on total failure.
    std::vector<uint8_t> read(const std::string& name) const;

    // Reads and, if necessary, decompresses a specific resolved TOC entry
    // directly - no name normalization or loose-file fallback. Useful for
    // tools that already have an exact entry (e.g. from entries()) and want
    // to bypass HOG_BigRead's caller-facing name-matching quirks (see
    // hogdump's --raw mode, for the one known real-archive entry,
    // "brgoodluck.ut", that keeps its extension in the TOC unlike its
    // siblings and would otherwise be unreachable through read()'s normal
    // stripping rule).
    std::vector<uint8_t> readEntry(const BigFileTocEntry& entry) const;

private:
    mutable std::ifstream m_stream;
    std::string m_path;
    std::string m_directory;
    std::vector<BigFileTocEntry> m_entries;
};

} // namespace neoslancer
