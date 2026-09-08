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
// payload]. Only the first 768 bytes (256 x 8-bit R,G,B triples, no
// alpha) are needed to colorize 8-bit indexed WinVFX sprites/fonts, and
// are all this parses. Validated empirically this session: rendered as a
// swatch strip, it shows the expected structure of a real hand-authored
// game palette (distinct hue ramps fading to black), and using it to
// colorize a real decoded FRONTEND.SPR shape produced a recognizable,
// correctly-toned image (not noise).
struct WinVfxPalette {
    std::array<std::array<uint8_t, 3>, 256> colors{}; // [index] = {R, G, B}
};

bool parseWinVfxPalette(const std::vector<uint8_t>& data, WinVfxPalette& out);

} // namespace neoslancer
