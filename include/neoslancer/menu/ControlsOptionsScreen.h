#pragma once

#include "neoslancer/GameConfig.h"
#include "neoslancer/WinVfxSprite.h"
#include "neoslancer/menu/AdjustableList.h"
#include "neoslancer/menu/MenuAssets.h"
#include "neoslancer/menu/MenuLayout.h"
#include "neoslancer/menu/MenuScreen.h"

#include <string>

namespace neoslancer {

// RunControlsOptionsScreen (screen ID 16). Confidence 3: confirms
// [KeyConfig]'s device-level flags (ForceFeedback/JoystickInvert/
// HatEnable/TwistEnable/controller) - confidence_db.md. This screen
// really reads/writes those via GameConfig/IniFile. What's NOT ported:
// the original's full interactive key-rebind flow (select a control,
// press a new key, per-action binding table with conflict detection) -
// that needs an actual per-action key-binding data model this project
// doesn't have yet, so only the device-level flags are editable here.
// Back-target (-> Options hub, screen 1) is our own inference.
//
// Loads its own sprite sheet, `interface\frntend6.spr` (a different
// file from the shared FRONTEND.SPR, with its own embedded palette -
// see WinVfxSprite.h), the same way OptionsMenuScreen loads frntend4.spr.
// Decompiling the real per-frame draw (Lancer.exe 0x42cd30) confirms
// the first 4 device-flag rows are drawn as a checkbox frame (shape 26,
// always) with a checkmark (shape 27) added inside when true - ported
// for those same 4 flags here, though positioned against this screen's
// own (not real-pixel-identical) list layout rather than the real
// screen's exact absolute coordinates, since this port's overall layout
// (title/body text, centered list) doesn't otherwise match the real
// screen's very different one (a full scrollable 12-row key-rebind
// list this port doesn't have a data model for - see above). No `.bik`
// is opened for a persistent background by this screen itself, but
// leaving to the Options hub plays the real optfade2.bik transition
// clip first (../StarLancer/reversing docs Pass 60), whose frozen final
// frame becomes MenuAssets::background - shared with every other screen
// reachable from the Main Menu, not owned per-screen.
class ControlsOptionsScreen : public MenuScreen {
public:
    ControlsOptionsScreen(std::string iniPath, const MenuAssets* assets);

    void onEnter(MenuManager& manager) override;
    void handleEvent(const SDL_Event& event, MenuManager& manager) override;
    bool update(float deltaSeconds, MenuManager& manager) override;
    void render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) override;

private:
    void save();
    void rebuildRows();
    void renderWithWinVfx(UIRenderer& renderer, int windowWidth, int windowHeight);
    void renderFallback(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight);

    std::string m_iniPath;
    const MenuAssets* m_assets;
    KeyConfigFlags m_flags;
    AdjustableList m_rows;

    WinVfxSprite m_sprite;
    bool m_spriteLoaded = false;
};

} // namespace neoslancer
