#pragma once

#include "neoslancer/Window.h"
#include "neoslancer/gfx/Font.h"
#include "neoslancer/gfx/UIRenderer.h"
#include "neoslancer/menu/MenuManager.h"

#include <memory>
#include <string>

namespace neoslancer {

// Top-level application: owns SDL/window/GL lifecycle and the main loop.
class Application {
public:
    Application() = default;
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // iniPath is threaded through to the options menu screens, which
    // read/write starlancer.ini directly (see SoundOptionsScreen etc.).
    // dataRoot is threaded through to VRRoomScreen, which resolves real
    // .bik movie files under it (cd1/cd2 subdirectories).
    bool init(const WindowConfig& config, const std::string& iniPath, const std::string& dataRoot);
    void run();
    void shutdown();

private:
    void handleEvents();
    void update(float deltaSeconds);
    void render();

    Window m_window;
    UIRenderer m_uiRenderer;
    Font m_font;
    std::unique_ptr<MenuManager> m_menuManager;

    bool m_running = false;
    bool m_initialized = false;
};

} // namespace neoslancer
