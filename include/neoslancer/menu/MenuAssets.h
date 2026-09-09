#pragma once

#include "neoslancer/BigFile.h"
#include "neoslancer/WinVfxFont.h"
#include "neoslancer/WinVfxPalette.h"
#include "neoslancer/WinVfxSprite.h"
#include "neoslancer/gfx/WinVfxRenderer.h"
#include "neoslancer/video/BinkVideoPlayer.h"

#include <string>

namespace neoslancer {

// Real WinVFX assets shared across every menu screen, loaded once
// (Application owns the instance) instead of each screen opening its own
// archive handle and re-parsing the same font/palette. `renderer` also
// holds the shared GL glyph/shape texture cache, so the same "N" glyph
// texture drawn on ten different screens is only ever built once.
//
// `loaded` is false (and every screen falls back to its existing SDL_ttf
// placeholder rendering) if RESOURCE.HOG or either resource can't be
// read - see MainMenuScreen's original fallback logic, which this
// generalizes.
struct MenuAssets {
    bool loaded = false;
    BigFileArchive archive;
    WinVfxFont font;
    WinVfxPalette palette;

    // A real decoded FRONTEND.SPR (see Pass 39 in the reversing docs -
    // this exact filename is only circumstantial evidence of being the
    // real menu sprite sheet, not code-level confirmed) - loaded once
    // here so every screen can use it for decorative icons rather than
    // each screen re-reading/re-parsing its own copy. `spriteLoaded` can
    // be false even when `loaded` is true (font+palette are the load-
    // critical resources; a missing/unparseable sprite just means no
    // icons, not a full fallback to SDL_ttf).
    bool spriteLoaded = false;
    WinVfxSprite sprite;

    // mutable: screens hold a `const MenuAssets*` (it's shared, read-only
    // data as far as they're concerned) but drawing still needs to build
    // and cache GL textures on first use - a caching implementation
    // detail, not logical state.
    mutable WinVfxRenderer renderer;

    // The Main Menu's background (see MainMenuScreen.h: the frozen final
    // frame of the intro's last clip, SPLASH TO MM.BIK) - shared here,
    // not owned per-screen, because no screen this port has traced
    // (RunOptionsMenuScreen, RunControlsOptionsScreen) ever opens its own
    // background video or explicitly clears/replaces this one; the real
    // menu system evidently just leaves it showing underneath every
    // screen reachable from the Main Menu without a movie of its own.
    // `backgroundLoaded` false means the file wasn't found - screens
    // should fall back to a plain background fill.
    bool backgroundLoaded = false;
    mutable BinkVideoPlayer background;
};

bool loadMenuAssets(const std::string& dataRoot, MenuAssets& out);

} // namespace neoslancer
