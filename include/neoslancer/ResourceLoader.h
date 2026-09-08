#pragma once

#include "neoslancer/BigFile.h"

#include <cstdint>
#include <string>
#include <vector>

namespace neoslancer {

// Mirrors LoadResourceFileBuffer's mod/dev-override behavior (Pass 28):
// a loose file dropped into the game-data directory with the requested
// resource's relative name silently takes priority over the packed
// archive's copy, checked BEFORE the archive is consulted at all - no
// flag or configuration needed. Only falls through to the archive
// (BigFileArchive::read, which has its own internal loose-file fallback
// for TOC misses) when no such loose file exists.
class ResourceLoader {
public:
    ResourceLoader(std::string dataRoot, const BigFileArchive& archive);

    std::vector<uint8_t> load(const std::string& relativeName) const;

private:
    std::string m_dataRoot;
    const BigFileArchive& m_archive;
};

} // namespace neoslancer
