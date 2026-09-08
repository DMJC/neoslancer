#pragma once

#include "neoslancer/BigFile.h"
#include "neoslancer/WinVfxFont.h"
#include "neoslancer/WinVfxPalette.h"
#include "neoslancer/WinVfxSprite.h"
#include "neoslancer/gfx/WinVfxRenderer.h"

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
};

bool loadMenuAssets(const std::string& dataRoot, MenuAssets& out);

} // namespace neoslancer
