#include "neoslancer/GameConfig.h"

#include "neoslancer/IniFile.h"

namespace neoslancer {

DeviceConfig loadDeviceConfig(const std::string& iniPath) {
    DeviceConfig config;

    IniFile ini;
    if (!ini.load(iniPath)) {
        return config;
    }

    config.renderDevice0 = ini.getString("Device", "renderdevice0", config.renderDevice0);
    config.renderDevice1 = ini.getString("Device", "renderdevice1", config.renderDevice1);
    config.device = ini.getInt("Device", "Device", config.device);
    config.xres = ini.getInt("Device", "Xres", config.xres);
    config.yres = ini.getInt("Device", "Yres", config.yres);
    config.windowed = ini.getInt("Device", "Windowed", config.windowed ? 1 : 0) != 0;
    config.textureDetail = ini.getInt("Device", "Tdetail", config.textureDetail);
    config.geometryDetail = ini.getInt("Device", "Gdetail", config.geometryDetail);
    config.lightmaps = ini.getInt("Device", "Lmaps", config.lightmaps ? 1 : 0) != 0;
    config.view = ini.getInt("Device", "View", config.view);
    config.transitions = ini.getInt("Device", "Transitions", config.transitions ? 1 : 0) != 0;
    config.gamma = ini.getInt("Device", "gamma", config.gamma);

    return config;
}

void saveDeviceConfig(const std::string& iniPath, const DeviceConfig& config) {
    IniFile ini;
    ini.load(iniPath); // ok if this fails - we'll just create a fresh file

    ini.setString("Device", "renderdevice0", config.renderDevice0);
    ini.setString("Device", "renderdevice1", config.renderDevice1);
    ini.setInt("Device", "Device", config.device);
    ini.setInt("Device", "Xres", config.xres);
    ini.setInt("Device", "Yres", config.yres);
    ini.setInt("Device", "Windowed", config.windowed ? 1 : 0);
    ini.setInt("Device", "Tdetail", config.textureDetail);
    ini.setInt("Device", "Gdetail", config.geometryDetail);
    ini.setInt("Device", "Lmaps", config.lightmaps ? 1 : 0);
    ini.setInt("Device", "View", config.view);
    ini.setInt("Device", "Transitions", config.transitions ? 1 : 0);
    ini.setInt("Device", "gamma", config.gamma);

    ini.save(iniPath);
}

SoundConfig loadSoundConfig(const std::string& iniPath) {
    SoundConfig config;

    IniFile ini;
    if (!ini.load(iniPath)) {
        return config;
    }

    config.provider3D = ini.getString("Sound", "3DProvider", config.provider3D);
    config.fxVolume = ini.getInt("Sound", "Fxvolume", config.fxVolume);
    config.musicVolume = ini.getInt("Sound", "Musicvolume", config.musicVolume);
    config.speechVolume = ini.getInt("Sound", "Speechvolume", config.speechVolume);
    config.masterVolume = ini.getInt("Sound", "Mastervolume", config.masterVolume);

    return config;
}

void saveSoundConfig(const std::string& iniPath, const SoundConfig& config) {
    IniFile ini;
    ini.load(iniPath);

    ini.setString("Sound", "3DProvider", config.provider3D);
    ini.setInt("Sound", "Fxvolume", config.fxVolume);
    ini.setInt("Sound", "Musicvolume", config.musicVolume);
    ini.setInt("Sound", "Speechvolume", config.speechVolume);
    ini.setInt("Sound", "Mastervolume", config.masterVolume);

    ini.save(iniPath);
}

KeyConfigFlags loadKeyConfigFlags(const std::string& iniPath) {
    KeyConfigFlags flags;

    IniFile ini;
    if (!ini.load(iniPath)) {
        return flags;
    }

    flags.forceFeedback = ini.getInt("KeyConfig", "ForceFeedback", flags.forceFeedback ? 1 : 0) != 0;
    flags.joystickInvert = ini.getInt("KeyConfig", "JoystickInvert", flags.joystickInvert ? 1 : 0) != 0;
    flags.hatEnable = ini.getInt("KeyConfig", "HatEnable", flags.hatEnable ? 1 : 0) != 0;
    flags.twistEnable = ini.getInt("KeyConfig", "TwistEnable", flags.twistEnable ? 1 : 0) != 0;
    flags.controller = ini.getInt("KeyConfig", "Controller", flags.controller);

    return flags;
}

void saveKeyConfigFlags(const std::string& iniPath, const KeyConfigFlags& flags) {
    IniFile ini;
    ini.load(iniPath);

    ini.setInt("KeyConfig", "ForceFeedback", flags.forceFeedback ? 1 : 0);
    ini.setInt("KeyConfig", "JoystickInvert", flags.joystickInvert ? 1 : 0);
    ini.setInt("KeyConfig", "HatEnable", flags.hatEnable ? 1 : 0);
    ini.setInt("KeyConfig", "TwistEnable", flags.twistEnable ? 1 : 0);
    ini.setInt("KeyConfig", "Controller", flags.controller);

    ini.save(iniPath);
}

} // namespace neoslancer
