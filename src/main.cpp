#include "neoslancer/Application.h"
#include "neoslancer/BigFile.h"
#include "neoslancer/CommandLine.h"
#include "neoslancer/GameConfig.h"
#include "neoslancer/GamePaths.h"

#include <cstdio>

int main(int argc, char** argv) {
    const neoslancer::CommandLineOptions options = neoslancer::parseCommandLine(argc, argv);

    const std::string dataRoot = neoslancer::resolveDataRoot();
    const neoslancer::GamePaths paths = neoslancer::buildGamePaths(dataRoot);
    const neoslancer::DeviceConfig device = neoslancer::loadDeviceConfig(paths.iniPath);

    std::printf("neoslancer: data root '%s'\n", paths.root.c_str());
    std::printf("neoslancer: ini path '%s' (%s)\n", paths.iniPath.c_str(),
                 device.xres != 0 ? "loaded or defaulted" : "load failed");
    if (options.credits) {
        std::printf("neoslancer: -credits requested (not implemented yet)\n");
    }
    if (options.greyscale) {
        std::printf("neoslancer: -greyscale requested (not implemented yet)\n");
    }

    // WinMain step 6 ("Resource load"): the original treats a failed
    // RESOURCE.HOG open as fatal ("main init: load failed on resource").
    // Nothing in neoslancer consumes archive resources yet, so a missing
    // archive is logged rather than treated as fatal for now.
    neoslancer::BigFileArchive resourceArchive;
    if (resourceArchive.open(paths.root + "/RESOURCE.HOG")) {
        std::printf("neoslancer: opened RESOURCE.HOG (%zu entries)\n", resourceArchive.entries().size());
    } else {
        std::printf("neoslancer: RESOURCE.HOG not found under data root (no assets loaded)\n");
    }

    neoslancer::Application app;

    neoslancer::WindowConfig config;
    config.title = "neoslancer";
    config.width = device.xres;
    config.height = device.yres;
    config.fullscreen = !device.windowed;
    config.gamma = device.gamma;

    if (!app.init(config, paths.iniPath, paths.root)) {
        return 1;
    }

    app.run();
    return 0;
}
