#include "neoslancer/Application.h"

int main(int /*argc*/, char** /*argv*/) {
    neoslancer::Application app;

    neoslancer::WindowConfig config;
    config.title = "neoslancer";
    config.width = 1280;
    config.height = 720;

    if (!app.init(config)) {
        return 1;
    }

    app.run();
    return 0;
}
