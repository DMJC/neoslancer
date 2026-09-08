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

} // namespace neoslancer
