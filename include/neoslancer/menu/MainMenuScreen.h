#pragma once

#include "neoslancer/menu/ButtonList.h"
#include "neoslancer/menu/MenuAssets.h"
#include "neoslancer/menu/MenuScreen.h"

#include <array>
#include <string>

namespace neoslancer {

// RunMainMenuScreen (screen ID 0). Confidence 5: the 3 real button
// hotspots below are read directly from the binary's own hit-test table
// (../StarLancer/reversing docs, "Document hotspot layouts for all 12
// menu screens" pass, table at 0x4e5b90) - x/y/w/h in the game's native
// 640x480 reference space, scaled to the actual window the same way
// VRRoomScreen does. Two more table entries exist (index 3/4) but
// dispatch to a hidden mission-29 trigger / are inert, per that same
// pass - not ported.
//
// What's NOT confirmed: WHICH FRONTEND.SPR shape (if any) is drawn in
// each of these 3 real hotspot rects - no widget/shape-index table for
// this exists in the docs (only the hit-test rects were recovered). This
// port tried FRONTEND.SPR shapes 18/19/20 as a size-proximity guess (they
// closely match the ~184-190x290px real rect dimensions) but ruled that
// guess out: decompiling VFX_shape_blit_unclipped (0x100035fc in
// WINVFX8.DLL) directly confirms this port's RLE decoder is byte-exact
// (literal-run/solid-run opcodes, count via >>1, dword-optimized copy
// loop - all present verbatim in the real disassembly), and testing
// shape 18 against ALL THREE global palettes that exist in the game
// (palette.ccb, palette3.ccb, softpal.ccb - correctly RefPack-
// decompressed) still produces incoherent, near-full-256-color-range
// noise rather than the handful-of-nearby-indices pattern real period
// dithering produces. So the decode and palette pipeline are correct;
// shapes 18/19/20 are simply the wrong assets for these buttons (likely
// some other screen's artwork, or content not meant to be flat-
// paletted). No sprite fill is drawn in the 3 real panels until the
// correct shape indices are identified - they render as real-font-
// labeled, real-position/size bordered hotspots only. The title position
// and the 2 extra buttons (Ship Interior demo, Quit) are our own
// additions, not in the original hotspot table.
class MainMenuScreen : public MenuScreen {
public:
    explicit MainMenuScreen(const MenuAssets* assets) : m_assets(assets) {}

    void onEnter(MenuManager& manager) override;
    void handleEvent(const SDL_Event& event, MenuManager& manager) override;
    void render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) override;

private:
    struct MainHotspot {
        int x, y, w, h;    // 640x480 reference space, confidence 5 (see class doc comment)
        int targetScreenId;
        const char* label;
    };
    static const std::array<MainHotspot, 3>& hotspots();

    struct ScaledRect {
        float x, y, w, h;
    };
    std::array<ScaledRect, 3> layoutHotspots(int windowWidth, int windowHeight) const;

    void renderWithWinVfx(UIRenderer& renderer, int windowWidth, int windowHeight);
    void renderFallback(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight);

    const MenuAssets* m_assets;
    ButtonList m_extraButtons; // Ship Interior demo / Quit - our own additions

    int m_mouseX = 0, m_mouseY = 0;
    int m_hoveredIndex = -1;
};

} // namespace neoslancer
