#pragma once

#include "neoslancer/BigFile.h"
#include "neoslancer/WinVfxFont.h"
#include "neoslancer/WinVfxPalette.h"
#include "neoslancer/WinVfxSprite.h"
#include "neoslancer/gfx/WinVfxRenderer.h"
#include "neoslancer/menu/ButtonList.h"
#include "neoslancer/menu/MenuScreen.h"

#include <string>

namespace neoslancer {

// RunMainMenuScreen (screen ID 0). Confidence 2: "hit-tests 3 button rects
// (-> screens 12/14/1)" (New Game / Multiplayer / Options) is a direct
// decompile fact (confidence_db.md). Exact button positions/graphics
// aren't decoded (see ButtonList's own note) - a 4th "Quit" button and
// Escape-to-quit are our own addition, not from the original (the real
// title screen quits via window-close, and separately hides a mission-
// select cheat code this port doesn't attempt to reproduce).
//
// Renders with the real WinVFX assets when they can be loaded from
// RESOURCE.HOG: starlancer's own handel.fnt bitmap font for all text, a
// real decoded FRONTEND.SPR shape as a decorative accent, and the real
// palette.ccb master palette to colorize it (see WinVfxRenderer.h for
// how - none of this was proven to be the ORIGINAL title screen's own
// asset choices/layout, since that widget-position data was never
// decompiled; these are real assets used in a layout of our own). Falls
// back to the SDL_ttf placeholder rendering used elsewhere in menu/ if
// the archive or any of these three resources can't be loaded, so the
// menu still works without a full game-data install.
class MainMenuScreen : public MenuScreen {
public:
    explicit MainMenuScreen(std::string dataRoot);

    void onEnter(MenuManager& manager) override;
    void handleEvent(const SDL_Event& event, MenuManager& manager) override;
    void render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) override;

private:
    void renderWithWinVfx(UIRenderer& renderer, int windowWidth, int windowHeight);
    void renderFallback(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight);

    std::string m_dataRoot;
    ButtonList m_buttons;

    bool m_assetsLoaded = false;
    bool m_loadAttempted = false;
    BigFileArchive m_archive;
    WinVfxFont m_font;
    WinVfxPalette m_palette;
    WinVfxSprite m_sprite;
    WinVfxRenderer m_winVfxRenderer;
};

} // namespace neoslancer
