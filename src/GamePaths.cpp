#include "neoslancer/GamePaths.h"

#include <climits>
#include <cstdlib>
#include <unistd.h>

namespace neoslancer {

std::string getExecutableDirectory() {
    char buffer[PATH_MAX];
    const ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (length <= 0) {
        return ".";
    }
    buffer[length] = '\0';

    std::string path(buffer);
    const size_t lastSlash = path.find_last_of('/');
    if (lastSlash == std::string::npos) {
        return ".";
    }
    return path.substr(0, lastSlash);
}

std::string resolveDataRoot() {
    if (const char* override = std::getenv("NEOSLANCER_DATA")) {
        return override;
    }
    return getExecutableDirectory();
}

GamePaths buildGamePaths(const std::string& dataRoot) {
    GamePaths paths;
    paths.root = dataRoot;
    paths.ships = dataRoot + "/SHIPS";
    paths.missiles = dataRoot + "/MISSILES";
    paths.guns = dataRoot + "/GUNS";
    paths.addOns = dataRoot + "/ADD_ONS";
    paths.iniPath = dataRoot + "/starlancer.ini";
    return paths;
}

} // namespace neoslancer
