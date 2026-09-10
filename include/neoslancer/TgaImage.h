#pragma once

#include "neoslancer/WinVfxPalette.h"

#include <cstdint>
#include <vector>

namespace neoslancer {

// StarLancer's `.tga` assets are genuine, unmodified Targa (TGA 1.0) -
// no custom framing at all (../StarLancer/reversing docs Pass 61,
// confidence 5: verified directly against real files' headers AND
// against the engine's own self-identifying loader functions,
// `SR_TGA_allocate_palette`/`SR_TGA_get_palette`). Loose copies under
// `RESOURCE/` are additionally wrapped in the same whole-file RefPack
// compression already used for `.spr`/`.fnt`/`.SHP` - the caller is
// expected to have already decompressed that (see RefPack.h), same as
// for those formats; the raw `cd1`/`cd2` disc-extracted copies are
// plain, uncompressed standard TGA and need no such unwrapping.
//
// Notable real usage this port cares about: `palette.tga`/`softpal.tga`
// (confirmed the ACTUAL master 256-color RGB palette source consumed by
// `InitializeGraphicsDevice` - Pass 61 corrects Pass 25/30/54's earlier
// attribution of this to `.ccb`, which turns out to hold different,
// still-unidentified data instead), `palette3.tga`/`oldpalette.tga`
// (same mechanism, other render contexts), plus splash screens,
// screenshot output, VR room backdrops, and `.bik` transition-clip
// poster-frame stills.
//
// Standard TGA color-mapped images (imageType 1/9) store their color
// map as BGR (or BGRA), not RGB - confirmed directly from
// `SR_TGA_allocate_palette`'s own read loop. parseTga resolves this
// (and every other format detail: RLE packets, 15/16/24/32-bit pixel
// and color-map depths, both vertical storage orders) into a single
// straightforward top-to-bottom RGBA8 buffer, so callers never need to
// think about the on-disk format at all.
struct TgaImage {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> rgba; // width*height*4 bytes, top-left origin, RGBA8, straight (non-premultiplied) alpha
};

bool parseTga(const std::vector<uint8_t>& data, TgaImage& out);

// Reads ONLY a color-mapped TGA's embedded color map, into a
// WinVfxPalette - never decodes the image's own pixel data at all.
// This is exactly what the real engine's SR_TGA_allocate_palette/
// SR_TGA_get_palette do (Pass 61): `palette.tga`/`softpal.tga`/
// `palette3.tga`/`oldpalette.tga` each happen to also contain a real,
// viewable image (a HUD/UI icon atlas, unrelated to the palette itself
// - see tgaviewer) that this function deliberately ignores, extracting
// just the 256-entry color map that's actually consumed as the game's
// master 8-bit palette. Unlike `.ccb`'s Block A (WinVfxPalette.h), a
// TGA color map is already full 8-bit precision - no 6-bit-to-8-bit
// scaling needed here. Returns false if the file has no color map at
// all (colorMapType != 1) or is truncated.
bool parseTgaPalette(const std::vector<uint8_t>& data, WinVfxPalette& out);

} // namespace neoslancer
