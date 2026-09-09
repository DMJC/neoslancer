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

    // The current menu background - shared, not owned per-screen, and
    // mutated in place by playMenuTransition() below rather than each
    // screen managing its own player. Starts as the frozen final frame
    // of the intro's last clip, SPLASH TO MM.BIK (see MainMenuScreen.h),
    // matching what real StarLancer shows before any menu navigation has
    // happened yet. `backgroundLoaded` false means no background could
    // be loaded at all - screens should fall back to a plain fill.
    // Both are mutable alongside `renderer` above for the same reason:
    // screens hold a `const MenuAssets*` and call playMenuTransition()
    // through it to swap in a new clip - "which clip is currently
    // loaded" is a caching/state detail, not something that changes a
    // screen's own logical read-only view of its assets.
    mutable bool backgroundLoaded = false;
    mutable BinkVideoPlayer background;

    // dataRoot is kept here (not re-plumbed into every screen
    // constructor) so playMenuTransition() can find loose `.bik` files
    // without every caller needing its own copy.
    std::string dataRoot;
};

bool loadMenuAssets(const std::string& dataRoot, MenuAssets& out);

// Real StarLancer plays a short fade/wipe `.bik` clip at most menu
// navigation edges rather than cutting instantly - e.g. Main Menu ->
// Options plays `main2opt.bik`, Options -> a sub-screen plays
// `optfade.bik`, and so on (../StarLancer/reversing docs Pass 60 has
// the full real call map: every `INTERFACE\*.bik` clip, which real
// function plays it, and in which direction - confidence 5, from
// direct get_xrefs_to results, not filename guesses). Screens call this
// right before manager.goTo() to start playing the real clip for that
// specific edge into the shared `background` (real-time, not skipped
// to its last frame like loadMenuAssets' initial load) - each screen
// that shares `background` must then call `background.update()` from
// its own MenuScreen::update() override every frame for the clip to
// actually advance, exactly like IntroScreen already does for the
// logo/splash sequence. Once the clip ends, BinkVideoPlayer holds its
// final frame automatically, becoming the destination screen's static
// backdrop - the same "frozen last frame" pattern already established
// for SPLASH TO MM.BIK, just re-triggered per navigation edge instead
// of once at startup. Loose `.bik` clip files for these live under
// dataRoot/INTERFACE/, unlike SPLASH TO MM.BIK which sits at dataRoot's
// own top level - both are searched. Returns false (leaving the
// current background untouched) if the named clip can't be found/
// opened, so a missing file degrades to an instant cut rather than a
// blank screen. Takes `const MenuAssets&` (not `&`) since screens only
// ever hold a const pointer - `background`/`backgroundLoaded` are
// mutable, same as `renderer`.
bool playMenuTransition(const MenuAssets& assets, const std::string& clipName);

} // namespace neoslancer
