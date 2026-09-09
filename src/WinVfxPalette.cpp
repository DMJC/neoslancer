#include "neoslancer/WinVfxPalette.h"

namespace neoslancer {

namespace {
// Block A stores classic 6-bit VGA DAC precision (every real .ccb file
// checked has every one of its 768 bytes <= 63, not 255) - scale up to
// 8-bit the same way the engine's own PaletteOverrideRecord r6/g6/b6
// fields do (<<2), filling the low bits from the high bits rather than
// leaving them zero so full-scale white (0x3f) maps to 0xff, not 0xfc.
uint8_t scale6To8(uint8_t v) {
    return static_cast<uint8_t>((v << 2) | (v >> 4));
}
} // namespace

bool parseWinVfxPalette(const std::vector<uint8_t>& data, WinVfxPalette& out) {
    if (data.size() < 768) {
        return false;
    }
    for (size_t i = 0; i < 256; ++i) {
        out.colors[i][0] = scale6To8(data[i * 3 + 0]);
        out.colors[i][1] = scale6To8(data[i * 3 + 1]);
        out.colors[i][2] = scale6To8(data[i * 3 + 2]);
    }
    return true;
}

} // namespace neoslancer
