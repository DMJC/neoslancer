#include "neoslancer/menu/VideoOptionsScreen.h"

#include "neoslancer/menu/MenuManager.h"
#include "neoslancer/menu/MenuScreenIds.h"

#include <algorithm>

namespace neoslancer {

VideoOptionsScreen::VideoOptionsScreen(std::string iniPath, const MenuAssets* assets)
    : m_iniPath(std::move(iniPath)), m_assets(assets) {}

void VideoOptionsScreen::save() {
    saveDeviceConfig(m_iniPath, m_config);
}

void VideoOptionsScreen::rebuildRows() {
    m_rows.setRows({
        {"RESOLUTION", [this]() { return std::to_string(m_config.xres) + "x" + std::to_string(m_config.yres); },
         [](int) { /* mode enumeration not ported yet - display only */ }},
        {"WINDOWED", [this]() { return m_config.windowed ? "YES" : "NO"; },
         [this](int) {
             m_config.windowed = !m_config.windowed;
             save();
         }},
        {"GAMMA", [this]() { return std::to_string(m_config.gamma); },
         [this](int dir) {
             m_config.gamma = std::clamp(m_config.gamma + dir * 5, 0, 200);
             save();
         }},
        {"TEXTURE DETAIL", [this]() { return std::to_string(m_config.textureDetail); },
         [this](int dir) {
             m_config.textureDetail = std::clamp(m_config.textureDetail + dir, 0, 3);
             save();
         }},
        {"GEOMETRY DETAIL", [this]() { return std::to_string(m_config.geometryDetail); },
         [this](int dir) {
             m_config.geometryDetail = std::clamp(m_config.geometryDetail + dir, 0, 3);
             save();
         }},
        {"LIGHTMAPS", [this]() { return m_config.lightmaps ? "ON" : "OFF"; },
         [this](int) {
             m_config.lightmaps = !m_config.lightmaps;
             save();
         }},
        {"TRANSITIONS", [this]() { return m_config.transitions ? "ON" : "OFF"; },
         [this](int) {
             m_config.transitions = !m_config.transitions;
             save();
         }},
    });
}

void VideoOptionsScreen::onEnter(MenuManager& manager) {
    (void)manager;
    m_config = loadDeviceConfig(m_iniPath);
    rebuildRows();
}

void VideoOptionsScreen::handleEvent(const SDL_Event& event, MenuManager& manager) {
    if (event.type == SDL_KEYDOWN &&
        (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_BACKSPACE)) {
        manager.goTo(MenuScreenId::OptionsMenu);
        return;
    }
    m_rows.handleEvent(event);
}

void VideoOptionsScreen::render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) {
    if (m_assets && m_assets->loaded) {
        renderWithWinVfx(renderer, windowWidth, windowHeight);
    } else {
        renderFallback(renderer, font, windowWidth, windowHeight);
    }
}

void VideoOptionsScreen::renderFallback(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) {
    renderer.drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight),
                       Color{0.02f, 0.03f, 0.08f, 1.0f});

    const std::string title = "VIDEO OPTIONS";
    int titleW = 0, titleH = 0;
    renderer.measureText(font, title, titleW, titleH);
    float cursorY = static_cast<float>(windowHeight) * 0.10f;
    renderer.drawText(font, title, (static_cast<float>(windowWidth) - static_cast<float>(titleW)) * 0.5f, cursorY,
                       Color{0.75f, 0.85f, 1.0f, 1.0f});
    cursorY += static_cast<float>(titleH) + 24.0f;

    const float width = 360.0f;
    const float x = (static_cast<float>(windowWidth) - width) * 0.5f;
    m_rows.render(renderer, font, x, cursorY, width, 40.0f, 8.0f);

    const std::string hint = "LEFT/RIGHT to adjust, ESC to go back (changes apply next launch)";
    int hintW = 0, hintH = 0;
    renderer.measureText(font, hint, hintW, hintH);
    renderer.drawText(font, hint, (static_cast<float>(windowWidth) - static_cast<float>(hintW)) * 0.5f,
                       static_cast<float>(windowHeight) - static_cast<float>(hintH) - 24.0f,
                       Color{0.5f, 0.5f, 0.55f, 1.0f});
}

void VideoOptionsScreen::renderWithWinVfx(UIRenderer& renderer, int windowWidth, int windowHeight) {
    renderer.drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight),
                       Color{0.02f, 0.03f, 0.08f, 1.0f});

    WinVfxRenderer& vfx = m_assets->renderer;
    const std::string title = "VIDEO OPTIONS";
    int titleW = 0, titleH = 0;
    vfx.measureText(m_assets->font, title, titleW, titleH);
    float cursorY = static_cast<float>(windowHeight) * 0.10f;
    const float titleX = (static_cast<float>(windowWidth) - static_cast<float>(titleW)) * 0.5f;
    vfx.drawText(renderer, m_assets->font, m_assets->palette, title, titleX, cursorY, Color{0.80f, 0.88f, 1.0f, 1.0f});
    if (m_assets->spriteLoaded) {
        vfx.drawTitleAccents(renderer, m_assets->sprite, 1, m_assets->palette, titleX, cursorY,
                             static_cast<float>(titleW), static_cast<float>(titleH));
    }
    cursorY += static_cast<float>(titleH) + 24.0f;

    const float width = 360.0f;
    const float x = (static_cast<float>(windowWidth) - width) * 0.5f;
    m_rows.layout(x, cursorY, width, 40.0f, 8.0f);

    const Color normalBg{0.10f, 0.12f, 0.20f, 0.85f};
    const Color selectedBg{0.20f, 0.35f, 0.55f, 0.95f};
    const Color labelTint{0.85f, 0.87f, 0.95f, 1.0f};
    const Color valueTint{0.55f, 0.85f, 0.65f, 1.0f};
    const auto& rects = m_rows.rects();
    const auto& rows = m_rows.rows();
    for (size_t i = 0; i < rows.size(); ++i) {
        const auto& r = rects[i];
        renderer.drawRect(r.x, r.y, r.w, r.h, static_cast<int>(i) == m_rows.selectedIndex() ? selectedBg : normalBg);

        const std::string value = rows[i].displayValue ? rows[i].displayValue() : "";
        int labelH = 0, labelW = 0;
        vfx.measureText(m_assets->font, rows[i].label, labelW, labelH);
        vfx.drawText(renderer, m_assets->font, m_assets->palette, rows[i].label, r.x + 16.0f,
                     r.y + (r.h - static_cast<float>(labelH)) * 0.5f, labelTint);

        int valueW = 0, valueH = 0;
        vfx.measureText(m_assets->font, value, valueW, valueH);
        vfx.drawText(renderer, m_assets->font, m_assets->palette, value,
                     r.x + r.w - static_cast<float>(valueW) - 16.0f, r.y + (r.h - static_cast<float>(valueH)) * 0.5f,
                     valueTint);
    }

    const std::string hint = "LEFT/RIGHT to adjust, ESC to go back (changes apply next launch)";
    int hintW = 0, hintH = 0;
    vfx.measureText(m_assets->font, hint, hintW, hintH);
    vfx.drawText(renderer, m_assets->font, m_assets->palette, hint,
                 (static_cast<float>(windowWidth) - static_cast<float>(hintW)) * 0.5f,
                 static_cast<float>(windowHeight) - static_cast<float>(hintH) - 24.0f, Color{0.5f, 0.5f, 0.55f, 1.0f});
}

} // namespace neoslancer
