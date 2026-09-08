#include "neoslancer/menu/MainMenuScreen.h"

#include "neoslancer/menu/MenuManager.h"
#include "neoslancer/menu/MenuScreenIds.h"

namespace neoslancer {

MainMenuScreen::MainMenuScreen(std::string dataRoot) : m_dataRoot(std::move(dataRoot)) {}

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

    if (!m_loadAttempted) {
        m_loadAttempted = true;
        if (m_archive.open(m_dataRoot + "/RESOURCE.HOG")) {
            const std::vector<uint8_t> fontData = m_archive.read("handel.fnt");
            const std::vector<uint8_t> paletteData = m_archive.read("palette.ccb");
            const std::vector<uint8_t> spriteData = m_archive.read("FRONTEND.SPR");
            m_assetsLoaded = !fontData.empty() && !paletteData.empty() && !spriteData.empty() &&
                              parseWinVfxFont(fontData, m_font) && parseWinVfxPalette(paletteData, m_palette) &&
                              parseWinVfxSprite(spriteData, m_sprite);
        }
    }
}

void MainMenuScreen::handleEvent(const SDL_Event& event, MenuManager& manager) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        manager.requestQuitApplication();
        return;
    }
    m_buttons.handleEvent(event);
}

void MainMenuScreen::render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) {
    if (m_assetsLoaded) {
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

    const Color titleTint{0.80f, 0.88f, 1.0f, 1.0f};
    const std::string title = "STARLANCER";
    int titleW = 0, titleH = 0;
    m_winVfxRenderer.measureText(m_font, title, titleW, titleH);
    const float titleX = (static_cast<float>(windowWidth) - static_cast<float>(titleW)) * 0.5f;
    const float titleY = static_cast<float>(windowHeight) * 0.16f;
    m_winVfxRenderer.drawText(renderer, m_font, m_palette, title, titleX, titleY, titleTint);

    // A real decoded FRONTEND.SPR shape (one of a set of 16 identically-
    // sized 30x31 icons) as a decorative accent beside the title - real
    // asset, our own placement (see class doc comment).
    int iconW = 0, iconH = 0;
    if (m_winVfxRenderer.shapeSize(m_sprite, 1, iconW, iconH)) {
        m_winVfxRenderer.drawShape(renderer, m_sprite, 1, m_palette,
                                    titleX - static_cast<float>(iconW) - 16.0f,
                                    titleY + (static_cast<float>(titleH) - static_cast<float>(iconH)) * 0.5f);
        m_winVfxRenderer.drawShape(renderer, m_sprite, 1, m_palette,
                                    titleX + static_cast<float>(titleW) + 16.0f,
                                    titleY + (static_cast<float>(titleH) - static_cast<float>(iconH)) * 0.5f);
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
        m_winVfxRenderer.measureText(m_font, buttons[i].label, labelW, labelH);
        const float textX = r.x + (r.w - static_cast<float>(labelW)) * 0.5f;
        const float textY = r.y + (r.h - static_cast<float>(labelH)) * 0.5f;
        m_winVfxRenderer.drawText(renderer, m_font, m_palette, buttons[i].label, textX, textY, textTint);
    }
}

} // namespace neoslancer
