#pragma once

#include "neoslancer/menu/MenuAssets.h"
#include "neoslancer/menu/MenuLayout.h"
#include "neoslancer/menu/MenuScreen.h"
#include "neoslancer/WinVfxSprite.h"

#include <array>

namespace neoslancer {

// RunOptionsMenuScreen (screen ID 1). Confidence 5 on the hotspot table
// (../StarLancer/reversing docs Pass 48) and on the resource/hover-glow
// details below (independently decompiled this pass: Lancer.exe
// 0x42a620 for the resource + hotspot dispatch, 0x42afb0 for the
// per-frame draw that reveals the hover-glow shapes/positions).
//
// Loads its own sprite sheet, `interface\frntend4.spr` - a DIFFERENT
// file from the shared FRONTEND.SPR in MenuAssets, with its own
// embedded palette (WinVfxSprite.h). No `.bik` video is opened by this
// screen at all (confirmed: no BinkOpen call anywhere in its
// decompile) - the background is simply whatever MenuAssets::background
// is already showing (the Main Menu's frozen video frame - see
// MainMenuScreen.h), left untouched.
//
// The 3 large category tiles show NO shape in their base state (no
// unconditional large-shape draw exists in the real per-frame function
// at all) - only real-font labels over the shared background, exactly
// like this port's Main Menu tiles. On hover, a glow shape (19/20/21)
// is drawn at a real, decompiled-not-guessed (x,y) near-but-not-exactly
// matching the tile's own position (confirmed via 3 consistent ~5-20px
// insets against the real hotspot table), at the shape's own native
// size (the real draw call passes no explicit width/height). The 3
// small nav buttons (Exit/dialog/dialog) work exactly like the Main
// Menu's Instant Action/Quit: shape 27 always drawn, shape 28
// additionally on hover, both real per-frame-function facts.
class OptionsMenuScreen : public MenuScreen {
public:
    explicit OptionsMenuScreen(const MenuAssets* assets) : m_assets(assets) {}

    void onEnter(MenuManager& manager) override;
    void handleEvent(const SDL_Event& event, MenuManager& manager) override;
    void render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) override;

private:
    struct Tile {
        int x, y, w, h;       // 640x480 reference space, confidence 5
        int glowShape;        // FRNTEND4.SPR shape index, confidence 5
        int glowX, glowY;     // hover-glow draw position, confidence 5 (native shape size, not stretched)
        int targetScreenId;
        const char* label;
    };
    struct NavButton {
        int x, y, w, h; // 640x480 reference space, confidence 5
        int targetScreenId;
        const char* label;
    };
    static const std::array<Tile, 3>& tiles();
    static const std::array<NavButton, 3>& navButtons();

    void renderWithWinVfx(UIRenderer& renderer, int windowWidth, int windowHeight);
    void renderFallback(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight);

    const MenuAssets* m_assets;
    WinVfxSprite m_sprite;
    bool m_spriteLoaded = false;

    int m_mouseX = 0, m_mouseY = 0;
    int m_hoveredTile = -1;
    int m_hoveredNav = -1;
};

} // namespace neoslancer
