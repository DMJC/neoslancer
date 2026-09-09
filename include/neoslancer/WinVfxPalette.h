#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace neoslancer {

// The engine's master 256-color palette, loaded from a `.ccb` resource
// (palette.ccb/softpal.ccb) via SR_CCB_load and shared between SurrenderLib
// (3D) and WinVFX (2D) - ../StarLancer/reversing/reverse_engineered_functions.md
// Pass 30, confidence 4 for the struct below (derived directly from
// SR_CCB_load's field-by-field construction).
//
// On-disk layout: [0x300 bytes: 256 RGB triples][0xc00 bytes: a second,
// structurally-confirmed-but-functionally-unconfirmed block, plausibly a
// precomputed lighting/shading ramp table][header scalars][variable
// payload]. Only the first 768 bytes (256 x 3 color-component bytes, no
// alpha) are needed to colorize 8-bit indexed WinVFX sprites/fonts, and
// are all this parses.
//
// Those 768 bytes are 6-bit VGA DAC precision, not 8-bit: every byte in
// every real .ccb file checked (palette.ccb/palette3.ccb/softpal.ccb) is
// <= 63, confirmed by scanning all 768 bytes of each - the same
// precision the engine's own per-shape PaletteOverrideRecord r6/g6/b6
// fields document explicitly. parseWinVfxPalette scales each component
// up to 8-bit (`(v<<2)|(v>>4)`) accordingly. Validated empirically
// against a live, Wine-run copy of the real game: sampled real skin-tone
// pixels from its Main Menu and found near-exact matches (color-distance
// ~37 out of a max ~195075) in softpal.ccb's scaled entries - using the
// raw, unscaled bytes instead produces washed-out, incoherent color
// (everything capped at 25% brightness) that looks like decode noise but
// isn't.
struct WinVfxPalette {
    std::array<std::array<uint8_t, 3>, 256> colors{}; // [index] = {R, G, B}
};

bool parseWinVfxPalette(const std::vector<uint8_t>& data, WinVfxPalette& out);

} // namespace neoslancer
