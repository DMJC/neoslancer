#include "neoslancer/menu/AdjustableList.h"

namespace neoslancer {

void AdjustableList::setRows(std::vector<AdjustableRow> rows) {
    m_rows = std::move(rows);
    m_lastRects.clear();
    if (m_selected >= static_cast<int>(m_rows.size())) {
        m_selected = 0;
    }
}

void AdjustableList::adjustSelected(int direction) {
    if (m_selected < 0 || m_selected >= static_cast<int>(m_rows.size())) {
        return;
    }
    if (m_rows[m_selected].adjust) {
        m_rows[m_selected].adjust(direction);
    }
}

void AdjustableList::handleEvent(const SDL_Event& event) {
    if (m_rows.empty()) {
        return;
    }

    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                m_selected = (m_selected - 1 + static_cast<int>(m_rows.size())) % static_cast<int>(m_rows.size());
                break;
            case SDLK_DOWN:
                m_selected = (m_selected + 1) % static_cast<int>(m_rows.size());
                break;
            case SDLK_LEFT:
                adjustSelected(-1);
                break;
            case SDLK_RIGHT:
                adjustSelected(1);
                break;
            default:
                break;
        }
    } else if (event.type == SDL_MOUSEMOTION) {
        for (size_t i = 0; i < m_lastRects.size(); ++i) {
            const Rect& r = m_lastRects[i];
            if (event.motion.x >= r.x && event.motion.x < r.x + r.w && event.motion.y >= r.y &&
                event.motion.y < r.y + r.h) {
                m_selected = static_cast<int>(i);
                break;
            }
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        for (size_t i = 0; i < m_lastRects.size(); ++i) {
            const Rect& r = m_lastRects[i];
            if (event.button.x >= r.x && event.button.x < r.x + r.w && event.button.y >= r.y &&
                event.button.y < r.y + r.h) {
                m_selected = static_cast<int>(i);
                const float mid = r.x + r.w * 0.5f;
                adjustSelected(event.button.x < mid ? -1 : 1);
                break;
            }
        }
    }
}

void AdjustableList::layout(float x, float y, float width, float itemHeight, float gap) {
    m_lastRects.clear();
    m_lastRects.reserve(m_rows.size());
    for (size_t i = 0; i < m_rows.size(); ++i) {
        const float itemY = y + static_cast<float>(i) * (itemHeight + gap);
        m_lastRects.push_back({x, itemY, width, itemHeight});
    }
}

void AdjustableList::render(UIRenderer& renderer, Font& font, float x, float y, float width, float itemHeight,
                             float gap) {
    layout(x, y, width, itemHeight, gap);

    const Color normalBg{0.10f, 0.12f, 0.20f, 0.85f};
    const Color selectedBg{0.20f, 0.35f, 0.55f, 0.95f};
    const Color labelColor{0.85f, 0.87f, 0.95f, 1.0f};
    const Color valueColor{0.6f, 0.9f, 0.7f, 1.0f};

    for (size_t i = 0; i < m_rows.size(); ++i) {
        const Rect& r = m_lastRects[i];
        renderer.drawRect(r.x, r.y, r.w, r.h, static_cast<int>(i) == m_selected ? selectedBg : normalBg);

        const std::string& label = m_rows[i].label;
        const std::string value = m_rows[i].displayValue ? m_rows[i].displayValue() : "";

        int labelH = 0, labelW = 0;
        renderer.measureText(font, label, labelW, labelH);
        renderer.drawText(font, label, r.x + 16.0f, r.y + (r.h - static_cast<float>(labelH)) * 0.5f, labelColor);

        int valueW = 0, valueH = 0;
        renderer.measureText(font, value, valueW, valueH);
        renderer.drawText(font, value, r.x + r.w - static_cast<float>(valueW) - 16.0f,
                           r.y + (r.h - static_cast<float>(valueH)) * 0.5f, valueColor);
    }
}

} // namespace neoslancer
