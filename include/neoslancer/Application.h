#pragma once

#include "neoslancer/Window.h"

namespace neoslancer {

// Top-level application: owns SDL/window/GL lifecycle and the main loop.
class Application {
public:
    Application() = default;
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool init(const WindowConfig& config);
    void run();
    void shutdown();

private:
    void handleEvents();
    void update(float deltaSeconds);
    void render();

    Window m_window;
    bool m_running = false;
    bool m_initialized = false;
};

} // namespace neoslancer
