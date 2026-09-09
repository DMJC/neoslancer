#pragma once

#include "neoslancer/menu/MenuAssets.h"
#include "neoslancer/menu/MenuLayout.h"
#include "neoslancer/menu/MenuScreen.h"

#include <array>

namespace neoslancer {

// RunMainMenuScreen (screen ID 0). Confidence 5 on every coordinate below
// (../StarLancer/reversing docs, table at 0x4e5b90, decoded fully in
// Pass 51): 3 large hit-test rects (New Game/Multiplayer/Options) plus 2
// small ones (Instant Action / a hidden "watch ending" mission-29 cheat)
// at the bottom - x/y/w/h in the game's native 640x480 reference space,
// letterboxed into the actual window via MenuLayout.h (the original
// never ran at any other aspect ratio).
//
// The background is NOT a FRONTEND.SPR shape - decompiling the screen's
// own per-frame draw function (Lancer.exe 0x4291b6) directly shows
// FRONTEND.SPR shapes 18/19/20 are only ever drawn while that specific
// hotspot is under the mouse (a hover-highlight overlay, never a
// permanent backdrop) - ported that way (see renderWithWinVfx). Byte-
// for-byte comparing a live Wine run of the real game against this
// port's own asset pipeline settled what the permanent backdrop
// actually is at rest: the frozen final frame of the intro sequence's
// last clip, SPLASH TO MM.BIK, which real StarLancer holds instead of
// discarding. Shared via MenuAssets::background/backgroundLoaded, not
// owned per-screen.
//
// Navigating to Options plays a real transition clip first
// (main2opt.bik - ../StarLancer/reversing docs Pass 60's real call
// map), same MenuAssets::playMenuTransition mechanism used throughout
// the Options hub - see OptionsMenuScreen.h. Single/Multi Player also
// have real clips (main2sin.bik/main2mul.bik) but aren't wired up here
// yet: their destinations (NewGameSetup/MultiplayerSetup) are still
// generic PlaceholderScreens that don't share MenuAssets::background,
// so playing a transition into one wouldn't have anywhere to land.
class MainMenuScreen : public MenuScreen {
public:
    explicit MainMenuScreen(const MenuAssets* assets) : m_assets(assets) {}

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

    void renderWithWinVfx(UIRenderer& renderer, int windowWidth, int windowHeight);
    void renderFallback(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight);

    const MenuAssets* m_assets;

    int m_mouseX = 0, m_mouseY = 0;
    int m_hoveredIndex = -1;
};

} // namespace neoslancer
