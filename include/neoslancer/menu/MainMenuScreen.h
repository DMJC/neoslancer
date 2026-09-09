#pragma once

#include "neoslancer/menu/MenuAssets.h"
#include "neoslancer/menu/MenuScreen.h"
#include "neoslancer/video/BinkVideoPlayer.h"

#include <array>
#include <string>

namespace neoslancer {

// RunMainMenuScreen (screen ID 0). Confidence 5 on every coordinate below
// (../StarLancer/reversing docs, table at 0x4e5b90, decoded fully in
// Pass 51): 3 large hit-test rects (New Game/Multiplayer/Options) plus 2
// small ones (Instant Action / a hidden "watch ending" mission-29 cheat)
// at the bottom - x/y/w/h in the game's native 640x480 reference space,
// letterboxed into the actual window at a fixed 4:3 aspect (the original
// never ran at any other ratio).
//
// The background is NOT a FRONTEND.SPR shape - decompiling the screen's
// own per-frame draw function (Lancer.exe 0x4291b6) directly shows
// FRONTEND.SPR shapes 18/19/20 are only ever drawn while that specific
// hotspot is under the mouse (a hover-highlight overlay, never a
// permanent backdrop); no unconditional "draw background" shape call
// exists in that function at all. Byte-for-byte comparing a live Wine
// run of the real game against this port's own asset pipeline settled
// it: the visible background is simply the frozen final frame of the
// intro sequence's last clip, SPLASH TO MM.BIK (already decoded
// correctly by this port's BinkVideoPlayer) - real StarLancer holds
// that frame as the menu's backdrop instead of discarding it. This
// screen owns its own BinkVideoPlayer for exactly that purpose; the
// hover-highlight shapes are not ported yet (unconfirmed which of
// FRONTEND.SPR's shapes is the correct glow asset - the size-proximity
// guess used earlier was never confirmed and is not re-used here).
class MainMenuScreen : public MenuScreen {
public:
    MainMenuScreen(std::string dataRoot, const MenuAssets* assets);

    void onEnter(MenuManager& manager) override;
    void handleEvent(const SDL_Event& event, MenuManager& manager) override;
    bool update(float deltaSeconds, MenuManager& manager) override;
    void render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) override;

private:
    struct Hotspot {
        int x, y, w, h; // 640x480 reference space, confidence 5 (see class doc comment)
        int targetScreenId;
        const char* label; // two lines, "\n"-separated where real; nullptr = no visible label (small icon buttons)
    };
    static const std::array<Hotspot, 5>& hotspots();

    struct ScaledRect {
        float x, y, w, h;
    };
    // Maps a 640x480-reference-space rect into a 4:3-letterboxed area of
    // the actual window (pillar/letterboxed, never stretched off-ratio).
    ScaledRect mapRect(int x, int y, int w, int h, int windowWidth, int windowHeight) const;
    ScaledRect viewport(int windowWidth, int windowHeight) const;

    void renderWithWinVfx(UIRenderer& renderer, int windowWidth, int windowHeight);
    void renderFallback(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight);

    std::string m_dataRoot;
    const MenuAssets* m_assets;
    BinkVideoPlayer m_background;

    int m_mouseX = 0, m_mouseY = 0;
    int m_hoveredIndex = -1;
};

} // namespace neoslancer
