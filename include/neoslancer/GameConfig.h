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
void saveDeviceConfig(const std::string& iniPath, const DeviceConfig& config);

// Mirrors starlancer.ini's [Sound] section. Confidence 3: keys confirmed
// directly (RunSoundOptionsScreen writes these via WritePrivateProfileStringA -
// see ../StarLancer/reversing/reverse_engineered_functions.md confidence_db.md,
// RunMenuScreenLoop screen ID 3). Default values are NOT confirmed against
// the binary's own defaults (Confidence 1, reasonable placeholders).
struct SoundConfig {
    std::string provider3D = "Software";
    int fxVolume = 100;
    int musicVolume = 100;
    int speechVolume = 100;
    int masterVolume = 100;
};

SoundConfig loadSoundConfig(const std::string& iniPath);
void saveSoundConfig(const std::string& iniPath, const SoundConfig& config);

// Mirrors starlancer.ini's [KeyConfig] section's device-level flags (not
// the per-action key bindings, which aren't ported yet). Confidence 3:
// keys confirmed directly (RunControlsOptionsScreen), and these exact
// values are also what a real on-disk starlancer.ini contains.
struct KeyConfigFlags {
    bool forceFeedback = true;
    bool joystickInvert = true;
    bool hatEnable = true;
    bool twistEnable = false;
    int controller = 1; // 0/1/2, plausibly keyboard/joystick/other device
};

KeyConfigFlags loadKeyConfigFlags(const std::string& iniPath);
void saveKeyConfigFlags(const std::string& iniPath, const KeyConfigFlags& flags);

} // namespace neoslancer
