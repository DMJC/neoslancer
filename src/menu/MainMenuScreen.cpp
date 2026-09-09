#include "neoslancer/menu/MainMenuScreen.h"

#include "neoslancer/menu/MenuManager.h"
#include "neoslancer/menu/MenuScreenIds.h"

namespace neoslancer {

namespace {
constexpr int kNoAction = -100;
constexpr int kQuitAction = -101;
} // namespace

const std::array<MainMenuScreen::Hotspot, 5>& MainMenuScreen::hotspots() {
    static const std::array<Hotspot, 5> table = {{
        {27, 123, 184, 290, MenuScreenId::NewGameSetup, "SINGLE\nPLAYER"},
        {203, 125, 184, 290, MenuScreenId::MultiplayerSetup, "MULTI\nPLAYER"},
        {421, 165, 184, 290, MenuScreenId::OptionsMenu, "GAME\nOPTIONS"},
        // Real hit-test rects (Pass 51): index 3 never reaches the action
        // switch in the original (absorbed by the hit-test loop, a no-op
        // there) and index 4 secretly re-runs mission 29 as a "watch
        // ending" cheat - neither is actually wired to the visible
        // INSTANT ACTION/QUIT text next to them. This port makes QUIT do
        // what it visibly says instead of silently doing nothing; INSTANT
        // ACTION has no mode implemented yet, so it's inert.
        {332, 441, 20, 15, kQuitAction, "QUIT"},
        {300, 441, 20, 15, kNoAction, "INSTANT ACTION"},
    }};
    return table;
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
        const int target = hotspots()[static_cast<size_t>(m_hoveredIndex)].targetScreenId;
        if (target == kQuitAction) {
            manager.requestQuitApplication();
        } else if (target != kNoAction) {
            if (target == MenuScreenId::OptionsMenu && m_assets) {
                playMenuTransition(*m_assets, "main2opt.bik");
            }
            manager.goTo(target);
        }
    }
}

bool MainMenuScreen::update(float deltaSeconds, MenuManager& manager) {
    (void)manager;
    if (m_assets && m_assets->backgroundLoaded) {
        m_assets->background.update(deltaSeconds);
    }
    return false;
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
                       Color{0.0f, 0.0f, 0.0f, 1.0f});

    const ScaledRect vp = menuViewport(windowWidth, windowHeight);
    if (m_assets && m_assets->backgroundLoaded) {
        renderer.drawTexture(m_assets->background.texture(), vp.x, vp.y, vp.w, vp.h);
    }

    const auto& table = hotspots();
    m_hoveredIndex = -1;
    for (size_t i = 0; i < table.size(); ++i) {
        const auto& h = table[i];
        const ScaledRect r = mapMenuRect(h.x, h.y, h.w, h.h, windowWidth, windowHeight);
        const bool hovered = static_cast<float>(m_mouseX) > r.x && static_cast<float>(m_mouseX) < r.x + r.w &&
                             static_cast<float>(m_mouseY) > r.y && static_cast<float>(m_mouseY) < r.y + r.h;
        if (hovered) {
            m_hoveredIndex = static_cast<int>(i);
        }
        renderer.drawRect(r.x, r.y, r.w, r.h,
                           hovered ? Color{0.9f, 0.7f, 0.2f, 0.55f} : Color{0.3f, 0.6f, 0.9f, 0.35f});

        int labelW = 0, labelH = 0;
        renderer.measureText(font, h.label, labelW, labelH);
        renderer.drawText(font, h.label, r.x + (r.w - static_cast<float>(labelW)) * 0.5f,
                           r.y + (r.h - static_cast<float>(labelH)) * 0.5f, Color{0.6f, 0.85f, 1.0f, 1.0f});
    }
}

void MainMenuScreen::renderWithWinVfx(UIRenderer& renderer, int windowWidth, int windowHeight) {
    renderer.drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight),
                       Color{0.0f, 0.0f, 0.0f, 1.0f});

    const ScaledRect vp = menuViewport(windowWidth, windowHeight);
    if (m_assets->backgroundLoaded) {
        renderer.drawTexture(m_assets->background.texture(), vp.x, vp.y, vp.w, vp.h);
    }

    WinVfxRenderer& vfx = m_assets->renderer;
    const auto& table = hotspots();
    m_hoveredIndex = -1;
    for (size_t i = 0; i < table.size(); ++i) {
        const auto& h = table[i];
        const ScaledRect r = mapMenuRect(h.x, h.y, h.w, h.h, windowWidth, windowHeight);
        const bool hovered = static_cast<float>(m_mouseX) > r.x && static_cast<float>(m_mouseX) < r.x + r.w &&
                             static_cast<float>(m_mouseY) > r.y && static_cast<float>(m_mouseY) < r.y + r.h;
        if (hovered) {
            m_hoveredIndex = static_cast<int>(i);
        }

        const Color tint = hovered ? Color{1.0f, 0.85f, 0.3f, 1.0f} : Color{0.35f, 0.75f, 1.0f, 1.0f};

        if (i < 3) {
            // The 3 large buttons: real StarLancer draws FRONTEND.SPR
            // shapes 18/19/20 here only while hovered (Lancer.exe
            // 0x4291b6 - see class doc comment), never as a permanent
            // background - confirmed by running the real game: with
            // nothing hovered these buttons show only the video frame
            // underneath. Drawn translucent, not opaque: rendered at
            // full opacity these shapes look like scrambled noise
            // (they're dithered photographic content, not a clean glow
            // asset), but blended softly over the video frame the way
            // the real engine's own alpha/tint state
            // (FUN_00480c40/FUN_00428410, not ported) likely intended,
            // they read as a plausible backlit highlight instead.
            if (hovered && m_assets->spriteLoaded) {
                vfx.drawShapeScaled(renderer, m_assets->sprite, static_cast<size_t>(18 + i), m_assets->palette, r.x,
                                    r.y, r.w, r.h, Color{1.0f, 1.0f, 1.0f, 0.35f});
            }

            const float labelY = r.y + r.h - 90.0f;
            int labelW = 0, labelH = 0;
            vfx.measureText(m_assets->font, h.label, labelW, labelH);
            // h.label may contain "\n" - draw line by line, centered.
            std::string line1 = h.label;
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
            vfx.drawText(renderer, m_assets->font, m_assets->palette, line1, cx - static_cast<float>(w1) * 0.5f,
                        labelY, tint);
            if (!line2.empty()) {
                vfx.drawText(renderer, m_assets->font, m_assets->palette, line2, cx - static_cast<float>(w2) * 0.5f,
                            labelY + static_cast<float>(hgt) + 2.0f, tint);
            }
            if (hovered) {
                const float th = 3.0f;
                renderer.drawRect(r.x, r.y, r.w, th, tint);
                renderer.drawRect(r.x, r.y + r.h - th, r.w, th, tint);
                renderer.drawRect(r.x, r.y, th, r.h, tint);
                renderer.drawRect(r.x + r.w - th, r.y, th, r.h, tint);
            }
        } else {
            // Instant Action / Quit: real per-frame draw (0x4291b6) always
            // draws shape 27 at exactly this rect, plus shape 28 in
            // addition while hovered - ported verbatim (both real,
            // confirmed shape indices and positions, not a guess).
            if (m_assets->spriteLoaded) {
                vfx.drawShapeScaled(renderer, m_assets->sprite, 27, m_assets->palette, r.x, r.y, r.w, r.h);
                if (hovered) {
                    vfx.drawShapeScaled(renderer, m_assets->sprite, 28, m_assets->palette, r.x, r.y, r.w, r.h);
                }
            } else {
                renderer.drawRect(r.x, r.y, r.w, r.h,
                                  hovered ? Color{1.0f, 0.75f, 0.2f, 1.0f} : Color{0.7f, 0.5f, 0.1f, 1.0f});
            }
            int labelW = 0, labelH = 0;
            vfx.measureText(m_assets->font, h.label, labelW, labelH);
            const float labelY = r.y + (r.h - static_cast<float>(labelH)) * 0.5f;
            if (i == 3) { // QUIT: label to the right of the box
                vfx.drawText(renderer, m_assets->font, m_assets->palette, h.label, r.x + r.w + 6.0f, labelY, tint);
            } else { // INSTANT ACTION: label to the left of the box
                vfx.drawText(renderer, m_assets->font, m_assets->palette, h.label,
                            r.x - static_cast<float>(labelW) - 6.0f, labelY, tint);
            }
        }
    }
}

} // namespace neoslancer
