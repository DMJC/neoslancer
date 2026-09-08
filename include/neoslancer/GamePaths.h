#pragma once

#include <string>

namespace neoslancer {

// Mirrors WinMain's path-setup step (reversing/reverse_engineered_functions.md
// lines 100-106): builds the game's data-file paths relative to a single
// root directory. The original always used the executable's own directory;
// we default to that too, but allow overriding it (see resolveDataRoot())
// since neoslancer ships without the original's proprietary game data and
// that data has to live somewhere the user points us at.
struct GamePaths {
    std::string root;
    std::string ships;    // real dir: SHIPS
    std::string missiles; // real dir: MISSILES
    std::string guns;     // real dir: GUNS
    std::string addOns;   // real dir: ADD_ONS
    std::string iniPath;  // starlancer.ini
};

// Executable's own directory (what the original resolves game-data paths
// relative to), via /proc/self/exe. Falls back to the current working
// directory if that can't be read.
std::string getExecutableDirectory();

// Picks the data root: the NEOSLANCER_DATA environment variable if set,
// otherwise the executable's directory (the original's behavior).
std::string resolveDataRoot();

GamePaths buildGamePaths(const std::string& dataRoot);

} // namespace neoslancer
