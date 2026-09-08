#include "neoslancer/BigFile.h"

#include "neoslancer/RefPack.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <strings.h> // strcasecmp

namespace neoslancer {

namespace {

uint32_t readU32BE(const uint8_t* p) {
    return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8) | static_cast<uint32_t>(p[3]);
}

constexpr uint32_t kMagicBIGF = 0x42494746;

std::string directoryOf(const std::string& path) {
    const size_t slash = path.find_last_of('/');
    return slash == std::string::npos ? "." : path.substr(0, slash);
}

// Strips any leading directory component and any 2-character extension
// starting with "ut" (HOG_BigRead's stripping rule; Pass 29 confidence 3
// on the mechanism, but the rule itself is what the decompile shows).
std::string normalizeRequestName(const std::string& name) {
    size_t start = name.find_last_of("/\\");
    start = (start == std::string::npos) ? 0 : start + 1;
    std::string base = name.substr(start);

    const size_t dot = base.find_last_of('.');
    if (dot != std::string::npos) {
        const std::string ext = base.substr(dot + 1);
        if (ext.size() >= 2 && ext[0] == 'u' && ext[1] == 't') {
            base = base.substr(0, dot);
        }
    }
    return base;
}

std::vector<uint8_t> readLooseFile(const std::string& path) {
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

} // namespace

BigFileArchive::~BigFileArchive() {
    close();
}

bool BigFileArchive::open(const std::string& path) {
    close();

    m_stream.open(path, std::ios::binary);
    if (!m_stream.is_open()) {
        return false;
    }

    uint8_t header[16];
    m_stream.read(reinterpret_cast<char*>(header), sizeof(header));
    if (!m_stream || readU32BE(header) != kMagicBIGF) {
        close();
        return false;
    }

    const uint32_t tocEntryCount = readU32BE(header + 8);
    const uint32_t tocSizeBytes = readU32BE(header + 12);

    std::vector<uint8_t> toc(tocSizeBytes);
    m_stream.seekg(0);
    m_stream.read(reinterpret_cast<char*>(toc.data()), tocSizeBytes);
    if (!m_stream) {
        close();
        return false;
    }

    m_entries.reserve(tocEntryCount);
    size_t cursor = 0x10;
    while (cursor + 8 < toc.size()) {
        BigFileTocEntry entry;
        entry.offset = readU32BE(&toc[cursor]);
        entry.size = readU32BE(&toc[cursor + 4]);

        const size_t nameStart = cursor + 8;
        const auto nameEndIt = std::find(toc.begin() + static_cast<long>(nameStart), toc.end(), '\0');
        if (nameEndIt == toc.end()) {
            break;
        }
        const size_t nameEnd = static_cast<size_t>(nameEndIt - toc.begin());
        entry.name.assign(reinterpret_cast<const char*>(&toc[nameStart]), nameEnd - nameStart);

        m_entries.push_back(std::move(entry));
        cursor = nameEnd + 1;
    }

    m_path = path;
    m_directory = directoryOf(path);
    return true;
}

void BigFileArchive::close() {
    if (m_stream.is_open()) {
        m_stream.close();
    }
    m_path.clear();
    m_directory.clear();
    m_entries.clear();
}

const BigFileTocEntry* BigFileArchive::findEntry(const std::string& name) const {
    for (const BigFileTocEntry& entry : m_entries) {
        if (strcasecmp(entry.name.c_str(), name.c_str()) == 0) {
            return &entry;
        }
    }
    return nullptr;
}

std::vector<uint8_t> BigFileArchive::read(const std::string& name) const {
    const std::string lookupName = normalizeRequestName(name);
    const BigFileTocEntry* entry = findEntry(lookupName);

    if (!entry) {
        // HOG_file_read fallback: a plain loose file, not inside any archive.
        return readLooseFile(m_directory + "/" + name);
    }

    return readEntry(*entry);
}

std::vector<uint8_t> BigFileArchive::readEntry(const BigFileTocEntry& entry) const {
    m_stream.seekg(entry.offset);
    uint8_t marker[2] = {0, 0};
    m_stream.read(reinterpret_cast<char*>(marker), sizeof(marker));
    m_stream.seekg(entry.offset);

    if (marker[0] == 0x10 && marker[1] == 0xFB) {
        uint8_t compressedHeader[5];
        m_stream.read(reinterpret_cast<char*>(compressedHeader), sizeof(compressedHeader));
        if (!m_stream) {
            return {};
        }
        const uint32_t decompressedSize =
            (static_cast<uint32_t>(compressedHeader[2]) << 16) |
            (static_cast<uint32_t>(compressedHeader[3]) << 8) | compressedHeader[4];

        // compressedHeader's 5-byte read already includes the 2-byte marker
        // (it re-reads from entry.offset), so only subtract it once.
        const size_t compressedPayloadSize = entry.size - sizeof(compressedHeader);
        std::vector<uint8_t> compressed(compressedPayloadSize);
        m_stream.read(reinterpret_cast<char*>(compressed.data()), static_cast<std::streamsize>(compressedPayloadSize));
        if (!m_stream) {
            return {};
        }

        std::vector<uint8_t> decompressed;
        if (!refpackDecompress(compressed.data(), compressed.size(), decompressedSize, decompressed)) {
            return {};
        }
        return decompressed;
    }

    std::vector<uint8_t> raw(entry.size);
    m_stream.read(reinterpret_cast<char*>(raw.data()), entry.size);
    if (!m_stream) {
        return {};
    }
    return raw;
}

} // namespace neoslancer
