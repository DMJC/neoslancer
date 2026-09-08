#include "neoslancer/menu/MenuAssets.h"

namespace neoslancer {

bool loadMenuAssets(const std::string& dataRoot, MenuAssets& out) {
    out.loaded = false;
    if (!out.archive.open(dataRoot + "/RESOURCE.HOG")) {
        return false;
    }

    const std::vector<uint8_t> fontData = out.archive.read("handel.fnt");
    const std::vector<uint8_t> paletteData = out.archive.read("palette.ccb");

    out.loaded = !fontData.empty() && !paletteData.empty() && parseWinVfxFont(fontData, out.font) &&
                 parseWinVfxPalette(paletteData, out.palette);

    const std::vector<uint8_t> spriteData = out.archive.read("FRONTEND.SPR");
    out.spriteLoaded = !spriteData.empty() && parseWinVfxSprite(spriteData, out.sprite);

    return out.loaded;
}

} // namespace neoslancer
