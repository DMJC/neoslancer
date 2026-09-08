#include "neoslancer/ResourceLoader.h"

#include <fstream>
#include <sys/stat.h>

namespace neoslancer {

namespace {

bool fileExistsOnDisk(const std::string& path) {
    struct stat info;
    return ::stat(path.c_str(), &info) == 0 && S_ISREG(info.st_mode);
}

std::vector<uint8_t> readWholeFile(const std::string& path) {
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

ResourceLoader::ResourceLoader(std::string dataRoot, const BigFileArchive& archive)
    : m_dataRoot(std::move(dataRoot)), m_archive(archive) {}

std::vector<uint8_t> ResourceLoader::load(const std::string& relativeName) const {
    const std::string loosePath = m_dataRoot + "/" + relativeName;
    if (fileExistsOnDisk(loosePath)) {
        return readWholeFile(loosePath);
    }
    return m_archive.read(relativeName);
}

} // namespace neoslancer
