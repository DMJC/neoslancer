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
    if (m_assets && m_assets->loaded) {
        renderWithWinVfx(renderer, windowWidth, windowHeight);
    } else {
        renderFallback(renderer, font, windowWidth, windowHeight);
    }
}

void SimpleMenuScreen::renderFallback(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) {
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

void SimpleMenuScreen::renderWithWinVfx(UIRenderer& renderer, int windowWidth, int windowHeight) {
    renderer.drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight),
                       Color{0.02f, 0.03f, 0.08f, 1.0f});

    WinVfxRenderer& vfx = m_assets->renderer;
    float cursorY = static_cast<float>(windowHeight) * 0.12f;

    if (!m_title.empty()) {
        int titleW = 0, titleH = 0;
        vfx.measureText(m_assets->font, m_title, titleW, titleH);
        const float titleX = (static_cast<float>(windowWidth) - static_cast<float>(titleW)) * 0.5f;
        vfx.drawText(renderer, m_assets->font, m_assets->palette, m_title, titleX, cursorY,
                     Color{0.80f, 0.88f, 1.0f, 1.0f});

        // Same real FRONTEND.SPR accent icon MainMenuScreen uses, for a
        // consistent look across every real-asset screen (see its own
        // doc comment - our own placement, not a decompiled layout).
        if (m_assets->spriteLoaded) {
            vfx.drawTitleAccents(renderer, m_assets->sprite, 1, m_assets->palette, titleX, cursorY,
                                 static_cast<float>(titleW), static_cast<float>(titleH));
        }

        cursorY += static_cast<float>(titleH) + 24.0f;
    }

    if (!m_body.empty()) {
        int bodyW = 0, bodyH = 0;
        vfx.measureText(m_assets->font, m_body, bodyW, bodyH);
        vfx.drawText(renderer, m_assets->font, m_assets->palette, m_body,
                     (static_cast<float>(windowWidth) - static_cast<float>(bodyW)) * 0.5f, cursorY,
                     Color{0.65f, 0.68f, 0.75f, 1.0f});
        cursorY += static_cast<float>(bodyH) + 24.0f;
    }

    const float buttonWidth = 260.0f;
    const float buttonHeight = 44.0f;
    const float gap = 12.0f;
    const float x = (static_cast<float>(windowWidth) - buttonWidth) * 0.5f;
    m_buttons.layout(x, cursorY, buttonWidth, buttonHeight, gap);

    const Color normalBg{0.10f, 0.12f, 0.20f, 0.85f};
    const Color selectedBg{0.20f, 0.35f, 0.55f, 0.95f};
    const Color textTint{0.90f, 0.92f, 1.0f, 1.0f};
    const auto& rects = m_buttons.rects();
    const auto& list = m_buttons.buttons();
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& r = rects[i];
        renderer.drawRect(r.x, r.y, r.w, r.h, static_cast<int>(i) == m_buttons.selectedIndex() ? selectedBg : normalBg);

        int labelW = 0, labelH = 0;
        vfx.measureText(m_assets->font, list[i].label, labelW, labelH);
        vfx.drawText(renderer, m_assets->font, m_assets->palette, list[i].label,
                     r.x + (r.w - static_cast<float>(labelW)) * 0.5f, r.y + (r.h - static_cast<float>(labelH)) * 0.5f,
                     textTint);
    }
}

} // namespace neoslancer
