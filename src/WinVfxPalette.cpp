#include "neoslancer/WinVfxPalette.h"

namespace neoslancer {

bool parseWinVfxPalette(const std::vector<uint8_t>& data, WinVfxPalette& out) {
    if (data.size() < 768) {
        return false;
    }
    for (size_t i = 0; i < 256; ++i) {
        out.colors[i][0] = data[i * 3 + 0];
        out.colors[i][1] = data[i * 3 + 1];
        out.colors[i][2] = data[i * 3 + 2];
    }
    return true;
}

} // namespace neoslancer
