#include "neoslancer/menu/SimpleMenuScreen.h"

namespace neoslancer {

void SimpleMenuScreen::handleEvent(const SDL_Event& event, MenuManager& manager) {
    (void)manager;
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE && onEscape) {
        onEscape();
        return;
    }
    m_buttons.handleEvent(event);
}

void SimpleMenuScreen::render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) {
    renderer.drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight),
                       Color{0.02f, 0.03f, 0.08f, 1.0f});

    float cursorY = static_cast<float>(windowHeight) * 0.12f;

    if (!m_title.empty()) {
        int titleW = 0, titleH = 0;
        renderer.measureText(font, m_title, titleW, titleH);
        renderer.drawText(font, m_title, (static_cast<float>(windowWidth) - static_cast<float>(titleW)) * 0.5f,
                           cursorY, Color{0.75f, 0.85f, 1.0f, 1.0f});
        cursorY += static_cast<float>(titleH) + 24.0f;
    }

    if (!m_body.empty()) {
        int bodyW = 0, bodyH = 0;
        renderer.measureText(font, m_body, bodyW, bodyH);
        renderer.drawText(font, m_body, (static_cast<float>(windowWidth) - static_cast<float>(bodyW)) * 0.5f,
                           cursorY, Color{0.7f, 0.7f, 0.75f, 1.0f});
        cursorY += static_cast<float>(bodyH) + 24.0f;
    }

    const float buttonWidth = 260.0f;
    const float buttonHeight = 44.0f;
    const float gap = 12.0f;
    const float x = (static_cast<float>(windowWidth) - buttonWidth) * 0.5f;
    m_buttons.render(renderer, font, x, cursorY, buttonWidth, buttonHeight, gap);
}

} // namespace neoslancer
