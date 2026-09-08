#pragma once

#include <string>

namespace neoslancer {

// Mirrors starlancer.ini's [Device] section, as read by WinMain's INI
// config-load step (reversing/reverse_engineered_functions.md lines
// 111-118) and cross-checked against a real starlancer.ini on disk
// (gamedata/StarLancer/starlancer.ini).
//
// Confidence 2: resolution (0x280x0x1e0 = 640x480) and gamma (100) defaults
// are read directly from the WinMain decompile. The other field defaults
// are Confidence 1 — plausible values taken from a real on-disk ini, not
// confirmed against the binary's own default constants.
struct DeviceConfig {
    std::string renderDevice0;
    std::string renderDevice1;
    int device = 0;
    int xres = 640;
    int yres = 480;
    bool windowed = false;
    int textureDetail = 1;
    int geometryDetail = 2;
    bool lightmaps = true;
    int view = 0;
    bool transitions = true;
    int gamma = 100;
};

// Loads starlancer.ini's [Device] section from iniPath. Returns default
// values (see DeviceConfig) if the file can't be read, matching the
// original engine's GetPrivateProfileIntA/StringA fallback-on-missing-key
// behavior rather than failing outright.
DeviceConfig loadDeviceConfig(const std::string& iniPath);

} // namespace neoslancer
