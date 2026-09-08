#include "neoslancer/menu/MainMenuScreen.h"

#include "neoslancer/menu/MenuManager.h"
#include "neoslancer/menu/MenuScreenIds.h"

namespace neoslancer {

void MainMenuScreen::onEnter(MenuManager& manager) {
    m_buttons.setButtons({
        {"NEW GAME", [&manager]() { manager.goTo(MenuScreenId::NewGameSetup); }},
        {"MULTIPLAYER", [&manager]() { manager.goTo(MenuScreenId::MultiplayerSetup); }},
        {"OPTIONS", [&manager]() { manager.goTo(MenuScreenId::OptionsMenu); }},
        // Not an original main-menu button - the real entry into the VR
        // loop is via RunMissionBriefingScreen (screen 7), which this port
        // hasn't built; this is a direct shortcut to it for now.
        {"SHIP INTERIOR (DEMO)", [&manager]() { manager.goTo(MenuScreenId::MissionBriefing); }},
        {"QUIT", [&manager]() { manager.requestQuitApplication(); }},
    });
}

void MainMenuScreen::handleEvent(const SDL_Event& event, MenuManager& manager) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        manager.requestQuitApplication();
        return;
    }
    m_buttons.handleEvent(event);
}

void MainMenuScreen::render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) {
    if (m_assets && m_assets->loaded) {
        renderWithWinVfx(renderer, windowWidth, windowHeight);
    } else {
        renderFallback(renderer, font, windowWidth, windowHeight);
    }
}

void MainMenuScreen::renderFallback(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) {
    renderer.drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight),
                       Color{0.02f, 0.03f, 0.08f, 1.0f});

    const std::string title = "STARLANCER";
    int titleW = 0, titleH = 0;
    renderer.measureText(font, title, titleW, titleH);
    renderer.drawText(font, title, (static_cast<float>(windowWidth) - static_cast<float>(titleW)) * 0.5f,
                       static_cast<float>(windowHeight) * 0.2f, Color{0.75f, 0.85f, 1.0f, 1.0f});

    const float buttonWidth = 340.0f;
    const float buttonHeight = 44.0f;
    const float gap = 12.0f;
    const float x = (static_cast<float>(windowWidth) - buttonWidth) * 0.5f;
    const float y = static_cast<float>(windowHeight) * 0.4f;
    m_buttons.render(renderer, font, x, y, buttonWidth, buttonHeight, gap);
}

void MainMenuScreen::renderWithWinVfx(UIRenderer& renderer, int windowWidth, int windowHeight) {
    renderer.drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight),
                       Color{0.02f, 0.03f, 0.08f, 1.0f});

    WinVfxRenderer& vfx = m_assets->renderer;
    const Color titleTint{0.80f, 0.88f, 1.0f, 1.0f};
    const std::string title = "STARLANCER";
    int titleW = 0, titleH = 0;
    vfx.measureText(m_assets->font, title, titleW, titleH);
    const float titleX = (static_cast<float>(windowWidth) - static_cast<float>(titleW)) * 0.5f;
    const float titleY = static_cast<float>(windowHeight) * 0.16f;
    vfx.drawText(renderer, m_assets->font, m_assets->palette, title, titleX, titleY, titleTint);

    // A real decoded FRONTEND.SPR shape (one of a set of 16 identically-
    // sized 30x31 icons) as a decorative accent beside the title - real
    // asset, our own placement (see class doc comment).
    if (m_assets->spriteLoaded) {
        vfx.drawTitleAccents(renderer, m_assets->sprite, 1, m_assets->palette, titleX, titleY,
                             static_cast<float>(titleW), static_cast<float>(titleH));
    }

    const float buttonWidth = 340.0f;
    const float buttonHeight = 44.0f;
    const float gap = 12.0f;
    const float x = (static_cast<float>(windowWidth) - buttonWidth) * 0.5f;
    const float y = static_cast<float>(windowHeight) * 0.4f;
    m_buttons.layout(x, y, buttonWidth, buttonHeight, gap);

    const Color normalBg{0.10f, 0.12f, 0.20f, 0.85f};
    const Color selectedBg{0.20f, 0.35f, 0.55f, 0.95f};
    const Color textTint{0.90f, 0.92f, 1.0f, 1.0f};
    const auto& rects = m_buttons.rects();
    const auto& buttons = m_buttons.buttons();
    for (size_t i = 0; i < buttons.size(); ++i) {
        const auto& r = rects[i];
        renderer.drawRect(r.x, r.y, r.w, r.h,
                           static_cast<int>(i) == m_buttons.selectedIndex() ? selectedBg : normalBg);

        int labelW = 0, labelH = 0;
        vfx.measureText(m_assets->font, buttons[i].label, labelW, labelH);
        const float textX = r.x + (r.w - static_cast<float>(labelW)) * 0.5f;
        const float textY = r.y + (r.h - static_cast<float>(labelH)) * 0.5f;
        vfx.drawText(renderer, m_assets->font, m_assets->palette, buttons[i].label, textX, textY, textTint);
    }
}

} // namespace neoslancer
