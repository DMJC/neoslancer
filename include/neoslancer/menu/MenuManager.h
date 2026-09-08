#pragma once

#include "neoslancer/gfx/Font.h"
#include "neoslancer/gfx/UIRenderer.h"
#include "neoslancer/menu/MenuScreen.h"

#include <SDL_events.h>

#include <memory>
#include <unordered_map>

namespace neoslancer {

// Ports RunMenuScreenLoop's dispatch shape (reversing/reverse_engineered_functions.md
// + confidence_db.md): a starting screen ID, one handler per ID, looping
// until the current handler signals exit. Screens themselves are our own
// C++ reimplementations, not decompiled field-by-field (see each screen's
// own header for how much of it is confirmed-real vs. an honest
// placeholder).
class MenuManager {
public:
    MenuManager(UIRenderer& renderer, Font& font);

    void registerScreen(int screenId, std::unique_ptr<MenuScreen> screen);
    void goTo(int screenId);

    void handleEvent(const SDL_Event& event);
    // Returns true once a screen has signalled "leave the whole menu
    // system" (e.g. NewGameSetupScreen finishing profile setup).
    bool update(float deltaSeconds);
    void render(int windowWidth, int windowHeight);

    void requestQuitApplication() { m_quitRequested = true; }
    bool quitRequested() const { return m_quitRequested; }

    UIRenderer& renderer() { return m_renderer; }
    Font& font() { return m_font; }

private:
    UIRenderer& m_renderer;
    Font& m_font;
    std::unordered_map<int, std::unique_ptr<MenuScreen>> m_screens;
    int m_currentId = -1;
    bool m_quitRequested = false;
};

} // namespace neoslancer
