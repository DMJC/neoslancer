#pragma once

#include "neoslancer/gfx/Font.h"
#include "neoslancer/gfx/UIRenderer.h"

#include <SDL_events.h>

namespace neoslancer {

class MenuManager;

// One RunMenuScreenLoop screen handler. update() returning true tells the
// MenuManager to leave the whole menu system (matching the original
// handler's "return nonzero" convention - see reverse_engineered_functions.md
// RunMenuScreenLoop: "continues looping as long as the called handler
// returns 0"). A screen normally transitions to a sibling screen by
// calling manager.goTo() itself and returning false (keep looping).
class MenuScreen {
public:
    virtual ~MenuScreen() = default;

    virtual void onEnter(MenuManager& manager) { (void)manager; }
    virtual void handleEvent(const SDL_Event& event, MenuManager& manager) = 0;
    virtual bool update(float deltaSeconds, MenuManager& manager) {
        (void)deltaSeconds;
        (void)manager;
        return false;
    }
    virtual void render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) = 0;
};

} // namespace neoslancer
