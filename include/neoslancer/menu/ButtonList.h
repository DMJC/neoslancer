#pragma once

#include "neoslancer/gfx/Font.h"
#include "neoslancer/gfx/UIRenderer.h"

#include <SDL_events.h>

#include <functional>
#include <string>
#include <vector>

namespace neoslancer {

struct MenuButton {
    std::string label;
    std::function<void()> onActivate;
};

// Shared vertical button-list widget: keyboard up/down + enter, mouse
// hover + click. Not part of the original engine - none of the 12
// RunMenuScreenLoop handlers' actual widget-position tables were
// decompiled (per METHODOLOGY, "hundreds of hardcoded pixel coordinates
// ... low value to reverse further"), so this is our own reasonable
// layout, reused across screens instead of hand-placing coordinates per
// screen.
class ButtonList {
public:
    struct Rect {
        float x, y, w, h;
    };

    void setButtons(std::vector<MenuButton> buttons);
    void handleEvent(const SDL_Event& event);
    void render(UIRenderer& renderer, Font& font, float x, float y, float width, float itemHeight, float gap);

    // Computes and stores the hit-test rects without drawing anything -
    // for screens (e.g. MainMenuScreen with real WinVFX assets) that want
    // ButtonList's input handling/layout but draw the buttons themselves.
    // render() calls this internally, so callers only need one or the
    // other, never both, in the same frame.
    void layout(float x, float y, float width, float itemHeight, float gap);

    const std::vector<Rect>& rects() const { return m_lastRects; }
    const std::vector<MenuButton>& buttons() const { return m_buttons; }
    int selectedIndex() const { return m_selected; }

private:
    std::vector<MenuButton> m_buttons;
    std::vector<Rect> m_lastRects; // from the last layout()/render(), used for hit-testing
    int m_selected = 0;

    void activate(int index);
};

} // namespace neoslancer
