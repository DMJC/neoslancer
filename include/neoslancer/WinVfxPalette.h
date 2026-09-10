#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace neoslancer {

// A plain 256-entry RGB color table - used for more than one real
// mechanism in this engine, not tied to a single file format.
//
// **Not** the menu system's master global palette, despite this
// project's own earlier passes assuming so: Pass 25/30/54 attributed
// that role to a `.ccb` resource's "Block A" (loaded via `SR_CCB_load`)
// and `parseWinVfxPalette` below was written against that assumption
// (6-bit-VGA-precision Block A bytes, scaled `(v<<2)|(v>>4)` to 8-bit -
// still accurate for what `.ccb` actually contains, see below). Pass 61
// corrected this: the real master palette
// `InitializeGraphicsDevice` loads at startup is a `.tga` file's
// embedded color map instead (`palette.tga`/`softpal.tga` - see
// TgaImage.h's `parseTgaPalette`, already full 8-bit precision, no
// scaling needed), confirmed directly from the engine's own self-
// identifying `SR_TGA_allocate_palette`/`SR_TGA_get_palette` loaders.
// `MenuAssets` reads the palette from there now, not from here.
// `.ccb`'s actual real purpose (what "Block A" really is, if not the
// master palette) is an open question again.
//
// What `parseWinVfxPalette`/this struct ARE still genuinely used and
// confirmed for: `.spr` files' own embedded per-file palette (Pass 59,
// WinVfxSprite.h - a real mechanism, confirmed pixel-exact against a
// real screenshot, storing the exact same 6-bit-VGA-precision 768 bytes
// this function expects, just embedded in a `.spr` file instead of a
// `.ccb` one) and the engine's separate per-shape `PaletteOverrideRecord`
// r6/g6/b6 fields (same precision, WinVfxSprite.h) - i.e. this format/
// scaling is real and used, just not for the ONE thing it was
// originally thought to be.
struct WinVfxPalette {
    std::array<std::array<uint8_t, 3>, 256> colors{}; // [index] = {R, G, B}
};

// Parses a `.ccb` resource's "Block A" (the first 768 bytes) as a
// 6-bit-VGA-precision RGB table, scaling each component up to 8-bit.
// See the struct comment above for what this is (and now isn't) used
// for in this port.
bool parseWinVfxPalette(const std::vector<uint8_t>& data, WinVfxPalette& out);

} // namespace neoslancer
