#pragma once

#include "neoslancer/menu/ButtonList.h"
#include "neoslancer/menu/MenuScreen.h"

namespace neoslancer {

// RunMainMenuScreen (screen ID 0). Confidence 2: "hit-tests 3 button rects
// (-> screens 12/14/1)" (New Game / Multiplayer / Options) is a direct
// decompile fact (confidence_db.md). Exact button positions/graphics
// aren't decoded (see ButtonList's own note) - a 4th "Quit" button and
// Escape-to-quit are our own addition, not from the original (the real
// title screen quits via window-close, and separately hides a mission-
// select cheat code this port doesn't attempt to reproduce).
class MainMenuScreen : public MenuScreen {
public:
    void onEnter(MenuManager& manager) override;
    void handleEvent(const SDL_Event& event, MenuManager& manager) override;
    void render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) override;

private:
    ButtonList m_buttons;
};

} // namespace neoslancer
