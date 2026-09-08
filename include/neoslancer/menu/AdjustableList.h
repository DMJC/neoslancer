#pragma once

#include "neoslancer/gfx/Font.h"
#include "neoslancer/gfx/UIRenderer.h"

#include <SDL_events.h>

#include <functional>
#include <string>
#include <vector>

namespace neoslancer {

struct AdjustableRow {
    std::string label;
    std::function<std::string()> displayValue;
    std::function<void(int direction)> adjust; // direction is -1 or +1
};

// A vertical list of "label: value" rows, adjustable with left/right (or
// clicking the left/right half of a row). Used by the options screens
// that edit real starlancer.ini values (Sound/Video/Controls) - see each
// screen's own header for which fields are confirmed-real vs. our own
// reasonable range choices.
class AdjustableList {
public:
    struct Rect {
        float x, y, w, h;
    };

    void setRows(std::vector<AdjustableRow> rows);
    void handleEvent(const SDL_Event& event);
    void render(UIRenderer& renderer, Font& font, float x, float y, float width, float itemHeight, float gap);

    // Computes and stores the hit-test rects without drawing - for
    // screens that want AdjustableList's input handling/layout but draw
    // the rows themselves (e.g. with real WinVFX fonts). render() calls
    // this internally.
    void layout(float x, float y, float width, float itemHeight, float gap);

    const std::vector<Rect>& rects() const { return m_lastRects; }
    const std::vector<AdjustableRow>& rows() const { return m_rows; }
    int selectedIndex() const { return m_selected; }

private:
    std::vector<AdjustableRow> m_rows;
    std::vector<Rect> m_lastRects;
    int m_selected = 0;

    void adjustSelected(int direction);
};

} // namespace neoslancer
