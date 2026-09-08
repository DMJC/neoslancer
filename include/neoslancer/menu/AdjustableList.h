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
    void setRows(std::vector<AdjustableRow> rows);
    void handleEvent(const SDL_Event& event);
    void render(UIRenderer& renderer, Font& font, float x, float y, float width, float itemHeight, float gap);

private:
    struct LaidOutRect {
        float x, y, w, h;
    };

    std::vector<AdjustableRow> m_rows;
    std::vector<LaidOutRect> m_lastRects;
    int m_selected = 0;

    void adjustSelected(int direction);
};

} // namespace neoslancer
