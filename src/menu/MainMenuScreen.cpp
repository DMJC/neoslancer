#include "neoslancer/menu/MainMenuScreen.h"

#include "neoslancer/menu/MenuManager.h"
#include "neoslancer/menu/MenuScreenIds.h"

namespace neoslancer {

namespace {
constexpr int kReferenceWidth = 640;
constexpr int kReferenceHeight = 480;
} // namespace

const std::array<MainMenuScreen::MainHotspot, 3>& MainMenuScreen::hotspots() {
    static const std::array<MainHotspot, 3> table = {{
        {27, 123, 184, 290, MenuScreenId::NewGameSetup, "NEW GAME"},
        {203, 125, 184, 290, MenuScreenId::MultiplayerSetup, "MULTIPLAYER"},
        {421, 165, 184, 290, MenuScreenId::OptionsMenu, "OPTIONS"},
    }};
    return table;
}

std::array<MainMenuScreen::ScaledRect, 3> MainMenuScreen::layoutHotspots(int windowWidth, int windowHeight) const {
    const float scaleX = static_cast<float>(windowWidth) / static_cast<float>(kReferenceWidth);
    const float scaleY = static_cast<float>(windowHeight) / static_cast<float>(kReferenceHeight);
    std::array<ScaledRect, 3> rects{};
    const auto& table = hotspots();
    for (size_t i = 0; i < table.size(); ++i) {
        rects[i] = {static_cast<float>(table[i].x) * scaleX, static_cast<float>(table[i].y) * scaleY,
                    static_cast<float>(table[i].w) * scaleX, static_cast<float>(table[i].h) * scaleY};
    }
    return rects;
}

void MainMenuScreen::onEnter(MenuManager& manager) {
    m_extraButtons.setButtons({
        // Not original buttons - the real entry into the VR loop is via
        // RunMissionBriefingScreen (screen 7), which this port hasn't
        // built; this is a direct shortcut to it for now.
        {"SHIP INTERIOR (DEMO)", [&manager]() { manager.goTo(MenuScreenId::MissionBriefing); }},
        {"QUIT", [&manager]() { manager.requestQuitApplication(); }},
    });
}

void MainMenuScreen::handleEvent(const SDL_Event& event, MenuManager& manager) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        manager.requestQuitApplication();
        return;
    }

    if (event.type == SDL_MOUSEMOTION) {
        m_mouseX = event.motion.x;
        m_mouseY = event.motion.y;
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        m_mouseX = event.button.x;
        m_mouseY = event.button.y;
    }

    const bool activate = (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) ||
                          (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) ||
                          (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN);
    if (activate && m_hoveredIndex >= 0) {
        manager.goTo(hotspots()[static_cast<size_t>(m_hoveredIndex)].targetScreenId);
        return;
    }

    m_extraButtons.handleEvent(event);
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
                       static_cast<float>(windowHeight) * 0.05f, Color{0.75f, 0.85f, 1.0f, 1.0f});

    const auto rects = layoutHotspots(windowWidth, windowHeight);
    const auto& table = hotspots();
    m_hoveredIndex = -1;
    for (size_t i = 0; i < table.size(); ++i) {
        const auto& r = rects[i];
        const bool hovered = static_cast<float>(m_mouseX) > r.x && static_cast<float>(m_mouseX) < r.x + r.w &&
                             static_cast<float>(m_mouseY) > r.y && static_cast<float>(m_mouseY) < r.y + r.h;
        if (hovered) {
            m_hoveredIndex = static_cast<int>(i);
        }
        renderer.drawRect(r.x, r.y, r.w, r.h,
                           hovered ? Color{0.20f, 0.35f, 0.55f, 0.95f} : Color{0.10f, 0.12f, 0.20f, 0.85f});

        int labelW = 0, labelH = 0;
        renderer.measureText(font, table[i].label, labelW, labelH);
        renderer.drawText(font, table[i].label, r.x + (r.w - static_cast<float>(labelW)) * 0.5f,
                           r.y + r.h - static_cast<float>(labelH) - 12.0f, Color{0.90f, 0.92f, 1.0f, 1.0f});
    }

    const float buttonWidth = 340.0f;
    const float buttonHeight = 40.0f;
    const float gap = 10.0f;
    const float bx = (static_cast<float>(windowWidth) - buttonWidth) * 0.5f;
    const float by = static_cast<float>(windowHeight) - (buttonHeight + gap) * 2.0f - 16.0f;
    m_extraButtons.render(renderer, font, bx, by, buttonWidth, buttonHeight, gap);
}

void MainMenuScreen::renderWithWinVfx(UIRenderer& renderer, int windowWidth, int windowHeight) {
    renderer.drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight),
                       Color{0.02f, 0.03f, 0.08f, 1.0f});

    WinVfxRenderer& vfx = m_assets->renderer;

    const std::string title = "STARLANCER";
    int titleW = 0, titleH = 0;
    vfx.measureText(m_assets->font, title, titleW, titleH);
    const float titleX = (static_cast<float>(windowWidth) - static_cast<float>(titleW)) * 0.5f;
    const float titleY = static_cast<float>(windowHeight) * 0.05f;
    vfx.drawText(renderer, m_assets->font, m_assets->palette, title, titleX, titleY,
                 Color{0.80f, 0.88f, 1.0f, 1.0f});
    if (m_assets->spriteLoaded) {
        vfx.drawTitleAccents(renderer, m_assets->sprite, 1, m_assets->palette, titleX, titleY,
                             static_cast<float>(titleW), static_cast<float>(titleH));
    }

    // The 3 real button hotspots (confidence 5 - see class doc comment).
    // No FRONTEND.SPR shape fill: the size-proximity guess (shapes
    // 18/19/20) was tested and ruled out (see class doc comment) -
    // decoder and palette pipeline are both confirmed correct, but those
    // specific shapes render as incoherent noise under every real global
    // palette, so they're simply the wrong assets. Bordered, real-font-
    // labeled hotspots only until the right shapes are identified.
    const auto rects = layoutHotspots(windowWidth, windowHeight);
    const auto& table = hotspots();
    m_hoveredIndex = -1;
    for (size_t i = 0; i < table.size(); ++i) {
        const auto& r = rects[i];
        const bool hovered = static_cast<float>(m_mouseX) > r.x && static_cast<float>(m_mouseX) < r.x + r.w &&
                             static_cast<float>(m_mouseY) > r.y && static_cast<float>(m_mouseY) < r.y + r.h;
        if (hovered) {
            m_hoveredIndex = static_cast<int>(i);
        }

        renderer.drawRect(r.x, r.y, r.w, r.h,
                           hovered ? Color{0.14f, 0.18f, 0.30f, 0.9f} : Color{0.07f, 0.08f, 0.14f, 0.85f});
        const Color frameTint = hovered ? Color{0.9f, 0.85f, 0.2f, 0.9f} : Color{0.4f, 0.7f, 1.0f, 0.6f};
        const float frameThickness = 3.0f;
        renderer.drawRect(r.x, r.y, r.w, frameThickness, frameTint);
        renderer.drawRect(r.x, r.y + r.h - frameThickness, r.w, frameThickness, frameTint);
        renderer.drawRect(r.x, r.y, frameThickness, r.h, frameTint);
        renderer.drawRect(r.x + r.w - frameThickness, r.y, frameThickness, r.h, frameTint);

        int labelW = 0, labelH = 0;
        vfx.measureText(m_assets->font, table[i].label, labelW, labelH);
        const float labelBarH = static_cast<float>(labelH) + 16.0f;
        renderer.drawRect(r.x, r.y + r.h - labelBarH, r.w, labelBarH, Color{0.0f, 0.0f, 0.0f, 0.6f});
        vfx.drawText(renderer, m_assets->font, m_assets->palette, table[i].label,
                     r.x + (r.w - static_cast<float>(labelW)) * 0.5f, r.y + r.h - labelBarH + 8.0f,
                     Color{0.92f, 0.94f, 1.0f, 1.0f});
    }

    const float buttonWidth = 340.0f;
    const float buttonHeight = 40.0f;
    const float gap = 10.0f;
    const float bx = (static_cast<float>(windowWidth) - buttonWidth) * 0.5f;
    const float by = static_cast<float>(windowHeight) - (buttonHeight + gap) * 2.0f - 16.0f;
    m_extraButtons.layout(bx, by, buttonWidth, buttonHeight, gap);

    const Color normalBg{0.10f, 0.12f, 0.20f, 0.85f};
    const Color selectedBg{0.20f, 0.35f, 0.55f, 0.95f};
    const Color textTint{0.90f, 0.92f, 1.0f, 1.0f};
    const auto& extraRects = m_extraButtons.rects();
    const auto& extraList = m_extraButtons.buttons();
    for (size_t i = 0; i < extraList.size(); ++i) {
        const auto& r = extraRects[i];
        renderer.drawRect(r.x, r.y, r.w, r.h,
                           static_cast<int>(i) == m_extraButtons.selectedIndex() ? selectedBg : normalBg);

        int labelW = 0, labelH = 0;
        vfx.measureText(m_assets->font, extraList[i].label, labelW, labelH);
        vfx.drawText(renderer, m_assets->font, m_assets->palette, extraList[i].label,
                     r.x + (r.w - static_cast<float>(labelW)) * 0.5f, r.y + (r.h - static_cast<float>(labelH)) * 0.5f,
                     textTint);
    }
}

} // namespace neoslancer
