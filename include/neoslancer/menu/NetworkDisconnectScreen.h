#pragma once

#include "neoslancer/menu/MenuScreen.h"

namespace neoslancer {

// RunNetworkDisconnectScreen (screen ID 8). Confidence 3: "Sleep(1000)
// then a network-cleanup call, returns to screen 3" (confidence_db.md) -
// ported as a real 1-second timed auto-transition, not a placeholder
// (there's no actual network layer yet to clean up, so that part is a
// no-op, but the "show a pause, then leave automatically after ~1s"
// behavior itself is genuinely reproduced).
class NetworkDisconnectScreen : public MenuScreen {
public:
    void onEnter(MenuManager& manager) override;
    void handleEvent(const SDL_Event& event, MenuManager& manager) override;
    bool update(float deltaSeconds, MenuManager& manager) override;
    void render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) override;

private:
    float m_elapsedSeconds = 0.0f;
};

} // namespace neoslancer
