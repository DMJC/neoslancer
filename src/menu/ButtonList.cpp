#include "neoslancer/menu/ButtonList.h"

namespace neoslancer {

void ButtonList::setButtons(std::vector<MenuButton> buttons) {
    m_buttons = std::move(buttons);
    m_lastRects.clear();
    if (m_selected >= static_cast<int>(m_buttons.size())) {
        m_selected = 0;
    }
}

void ButtonList::activate(int index) {
    if (index < 0 || index >= static_cast<int>(m_buttons.size())) {
        return;
    }
    m_selected = index;
    if (m_buttons[index].onActivate) {
        m_buttons[index].onActivate();
    }
}

void ButtonList::handleEvent(const SDL_Event& event) {
    if (m_buttons.empty()) {
        return;
    }

    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                m_selected = (m_selected - 1 + static_cast<int>(m_buttons.size())) % static_cast<int>(m_buttons.size());
                break;
            case SDLK_DOWN:
                m_selected = (m_selected + 1) % static_cast<int>(m_buttons.size());
                break;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
            case SDLK_SPACE:
                activate(m_selected);
                break;
            default:
                break;
        }
    } else if (event.type == SDL_MOUSEMOTION) {
        for (size_t i = 0; i < m_lastRects.size(); ++i) {
            const LaidOutRect& r = m_lastRects[i];
            if (event.motion.x >= r.x && event.motion.x < r.x + r.w && event.motion.y >= r.y &&
                event.motion.y < r.y + r.h) {
                m_selected = static_cast<int>(i);
                break;
            }
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        for (size_t i = 0; i < m_lastRects.size(); ++i) {
            const LaidOutRect& r = m_lastRects[i];
            if (event.button.x >= r.x && event.button.x < r.x + r.w && event.button.y >= r.y &&
                event.button.y < r.y + r.h) {
                activate(static_cast<int>(i));
                break;
            }
        }
    }
}

void ButtonList::render(UIRenderer& renderer, Font& font, float x, float y, float width, float itemHeight,
                         float gap) {
    m_lastRects.clear();
    m_lastRects.reserve(m_buttons.size());

    const Color normalBg{0.10f, 0.12f, 0.20f, 0.85f};
    const Color selectedBg{0.20f, 0.35f, 0.55f, 0.95f};
    const Color textColor{0.90f, 0.92f, 1.0f, 1.0f};

    for (size_t i = 0; i < m_buttons.size(); ++i) {
        const float itemY = y + static_cast<float>(i) * (itemHeight + gap);
        renderer.drawRect(x, itemY, width, itemHeight,
                           static_cast<int>(i) == m_selected ? selectedBg : normalBg);

        int textW = 0, textH = 0;
        renderer.measureText(font, m_buttons[i].label, textW, textH);
        const float textX = x + (width - static_cast<float>(textW)) * 0.5f;
        const float textY = itemY + (itemHeight - static_cast<float>(textH)) * 0.5f;
        renderer.drawText(font, m_buttons[i].label, textX, textY, textColor);

        m_lastRects.push_back({x, itemY, width, itemHeight});
    }
}

} // namespace neoslancer
