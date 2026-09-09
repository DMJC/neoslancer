#include "neoslancer/menu/OptionsMenuScreen.h"

#include "neoslancer/menu/MenuManager.h"
#include "neoslancer/menu/MenuScreenIds.h"

namespace neoslancer {

namespace {
constexpr int kNoAction = -100;
} // namespace

const std::array<OptionsMenuScreen::Tile, 3>& OptionsMenuScreen::tiles() {
    static const std::array<Tile, 3> table = {{
        {30, 165, 152, 127, 19, 35, 155, MenuScreenId::SoundOptions, "SOUND\nOPTIONS"},
        {219, 165, 152, 127, 20, 202, 160, MenuScreenId::ControlsOptions, "CONTROLS\nOPTIONS"},
        {408, 165, 152, 127, 21, 392, 161, MenuScreenId::VideoOptions, "VIDEO\nOPTIONS"},
    }};
    return table;
}

const std::array<OptionsMenuScreen::NavButton, 3>& OptionsMenuScreen::navButtons() {
    static const std::array<NavButton, 3> table = {{
        {292, 441, 25, 16, MenuScreenId::MainMenu, "EXIT"},
        // Real targets (Pass 48): a confirm-quit dialog and an
        // unidentified helper (FUN_0042a520) - neither dialog is built
        // in this port yet, so both stay visually real (position/icon)
        // but inert rather than guessing their behavior.
        {324, 441, 25, 16, kNoAction, nullptr},
        {292, 421, 25, 16, kNoAction, nullptr},
    }};
    return table;
}

void OptionsMenuScreen::onEnter(MenuManager& manager) {
    (void)manager;
    if (m_assets) {
        const std::vector<uint8_t> data = m_assets->archive.read("frntend4.spr");
        m_spriteLoaded = !data.empty() && parseWinVfxSprite(data, m_sprite);
    }
}

void OptionsMenuScreen::handleEvent(const SDL_Event& event, MenuManager& manager) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        manager.goTo(MenuScreenId::MainMenu);
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
    if (!activate) {
        return;
    }
    if (m_hoveredTile >= 0) {
        manager.goTo(tiles()[static_cast<size_t>(m_hoveredTile)].targetScreenId);
    } else if (m_hoveredNav >= 0) {
        const int target = navButtons()[static_cast<size_t>(m_hoveredNav)].targetScreenId;
        if (target != kNoAction) {
            manager.goTo(target);
        }
    }
}

void OptionsMenuScreen::render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) {
    if (m_assets && m_assets->loaded) {
        renderWithWinVfx(renderer, windowWidth, windowHeight);
    } else {
        renderFallback(renderer, font, windowWidth, windowHeight);
    }
}

void OptionsMenuScreen::renderFallback(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) {
    renderer.drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight),
                       Color{0.0f, 0.0f, 0.0f, 1.0f});
    if (m_assets && m_assets->backgroundLoaded) {
        const ScaledRect vp = menuViewport(windowWidth, windowHeight);
        renderer.drawTexture(m_assets->background.texture(), vp.x, vp.y, vp.w, vp.h);
    }

    m_hoveredTile = -1;
    for (size_t i = 0; i < tiles().size(); ++i) {
        const auto& t = tiles()[i];
        const ScaledRect r = mapMenuRect(t.x, t.y, t.w, t.h, windowWidth, windowHeight);
        const bool hovered = static_cast<float>(m_mouseX) > r.x && static_cast<float>(m_mouseX) < r.x + r.w &&
                             static_cast<float>(m_mouseY) > r.y && static_cast<float>(m_mouseY) < r.y + r.h;
        if (hovered) {
            m_hoveredTile = static_cast<int>(i);
        }
        renderer.drawRect(r.x, r.y, r.w, r.h,
                           hovered ? Color{0.9f, 0.7f, 0.2f, 0.55f} : Color{0.3f, 0.6f, 0.9f, 0.35f});
        int labelW = 0, labelH = 0;
        renderer.measureText(font, t.label, labelW, labelH);
        renderer.drawText(font, t.label, r.x + (r.w - static_cast<float>(labelW)) * 0.5f,
                           r.y + (r.h - static_cast<float>(labelH)) * 0.5f, Color{0.6f, 0.85f, 1.0f, 1.0f});
    }

    m_hoveredNav = -1;
    for (size_t i = 0; i < navButtons().size(); ++i) {
        const auto& n = navButtons()[i];
        const ScaledRect r = mapMenuRect(n.x, n.y, n.w, n.h, windowWidth, windowHeight);
        const bool hovered = static_cast<float>(m_mouseX) > r.x && static_cast<float>(m_mouseX) < r.x + r.w &&
                             static_cast<float>(m_mouseY) > r.y && static_cast<float>(m_mouseY) < r.y + r.h;
        if (hovered) {
            m_hoveredNav = static_cast<int>(i);
        }
        renderer.drawRect(r.x, r.y, r.w, r.h,
                           hovered ? Color{1.0f, 0.75f, 0.2f, 1.0f} : Color{0.7f, 0.5f, 0.1f, 1.0f});
        if (n.label) {
            int labelW = 0, labelH = 0;
            renderer.measureText(font, n.label, labelW, labelH);
            renderer.drawText(font, n.label, r.x - static_cast<float>(labelW) - 6.0f,
                              r.y + (r.h - static_cast<float>(labelH)) * 0.5f, Color{0.85f, 0.85f, 0.9f, 1.0f});
        }
    }
}

void OptionsMenuScreen::renderWithWinVfx(UIRenderer& renderer, int windowWidth, int windowHeight) {
    renderer.drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight),
                       Color{0.0f, 0.0f, 0.0f, 1.0f});
    if (m_assets->backgroundLoaded) {
        const ScaledRect vp = menuViewport(windowWidth, windowHeight);
        renderer.drawTexture(m_assets->background.texture(), vp.x, vp.y, vp.w, vp.h);
    }

    WinVfxRenderer& vfx = m_assets->renderer;

    m_hoveredTile = -1;
    for (size_t i = 0; i < tiles().size(); ++i) {
        const auto& t = tiles()[i];
        const ScaledRect r = mapMenuRect(t.x, t.y, t.w, t.h, windowWidth, windowHeight);
        const bool hovered = static_cast<float>(m_mouseX) > r.x && static_cast<float>(m_mouseX) < r.x + r.w &&
                             static_cast<float>(m_mouseY) > r.y && static_cast<float>(m_mouseY) < r.y + r.h;
        if (hovered) {
            m_hoveredTile = static_cast<int>(i);
        }

        const Color tint = hovered ? Color{1.0f, 0.85f, 0.3f, 1.0f} : Color{0.35f, 0.75f, 1.0f, 1.0f};

        if (hovered && m_spriteLoaded) {
            int sw = 0, sh = 0;
            if (vfx.shapeSize(m_sprite, static_cast<size_t>(t.glowShape), sw, sh)) {
                const ScaledRect glow = mapMenuRect(t.glowX, t.glowY, sw, sh, windowWidth, windowHeight);
                vfx.drawShapeScaled(renderer, m_sprite, static_cast<size_t>(t.glowShape), m_assets->palette, glow.x,
                                    glow.y, glow.w, glow.h, Color{1.0f, 1.0f, 1.0f, 0.7f});
            }
        }

        if (hovered) {
            const float th = 3.0f;
            renderer.drawRect(r.x, r.y, r.w, th, tint);
            renderer.drawRect(r.x, r.y + r.h - th, r.w, th, tint);
            renderer.drawRect(r.x, r.y, th, r.h, tint);
            renderer.drawRect(r.x + r.w - th, r.y, th, r.h, tint);
        }

        std::string line1 = t.label;
        std::string line2;
        const size_t nl = line1.find('\n');
        if (nl != std::string::npos) {
            line2 = line1.substr(nl + 1);
            line1 = line1.substr(0, nl);
        }
        int w1 = 0, hgt = 0, w2 = 0;
        vfx.measureText(m_assets->font, line1, w1, hgt);
        vfx.measureText(m_assets->font, line2, w2, hgt);
        const float cx = r.x + r.w * 0.5f;
        const float labelY = r.y + r.h - static_cast<float>(hgt) * 2.0f - 8.0f;
        vfx.drawText(renderer, m_assets->font, m_assets->palette, line1, cx - static_cast<float>(w1) * 0.5f, labelY,
                    tint);
        vfx.drawText(renderer, m_assets->font, m_assets->palette, line2, cx - static_cast<float>(w2) * 0.5f,
                    labelY + static_cast<float>(hgt) + 2.0f, tint);
    }

    m_hoveredNav = -1;
    for (size_t i = 0; i < navButtons().size(); ++i) {
        const auto& n = navButtons()[i];
        const ScaledRect r = mapMenuRect(n.x, n.y, n.w, n.h, windowWidth, windowHeight);
        const bool hovered = static_cast<float>(m_mouseX) > r.x && static_cast<float>(m_mouseX) < r.x + r.w &&
                             static_cast<float>(m_mouseY) > r.y && static_cast<float>(m_mouseY) < r.y + r.h;
        if (hovered) {
            m_hoveredNav = static_cast<int>(i);
        }

        if (m_spriteLoaded) {
            vfx.drawShapeScaled(renderer, m_sprite, 27, m_assets->palette, r.x, r.y, r.w, r.h);
            if (hovered) {
                vfx.drawShapeScaled(renderer, m_sprite, 28, m_assets->palette, r.x, r.y, r.w, r.h);
            }
        } else {
            renderer.drawRect(r.x, r.y, r.w, r.h,
                              hovered ? Color{1.0f, 0.75f, 0.2f, 1.0f} : Color{0.7f, 0.5f, 0.1f, 1.0f});
        }
        if (n.label) {
            int labelW = 0, labelH = 0;
            vfx.measureText(m_assets->font, n.label, labelW, labelH);
            vfx.drawText(renderer, m_assets->font, m_assets->palette, n.label, r.x - static_cast<float>(labelW) - 6.0f,
                        r.y + (r.h - static_cast<float>(labelH)) * 0.5f, Color{0.85f, 0.85f, 0.9f, 1.0f});
        }
    }
}

} // namespace neoslancer
