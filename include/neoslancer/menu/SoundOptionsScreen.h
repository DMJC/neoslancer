#pragma once

#include "neoslancer/GameConfig.h"
#include "neoslancer/menu/AdjustableList.h"
#include "neoslancer/menu/MenuAssets.h"
#include "neoslancer/menu/MenuLayout.h"
#include "neoslancer/menu/MenuScreen.h"

#include <string>

namespace neoslancer {

// RunSoundOptionsScreen (screen ID 3). Confidence 3: writes [Sound]
// 3DProvider/Fxvolume/Musicvolume/Speechvolume/Mastervolume directly to
// starlancer.ini (confidence_db.md) - this is genuinely ported, real
// behavior, not a placeholder: adjusting a row here writes straight back
// to the real ini via GameConfig/IniFile. The 3D-provider CHOICES offered
// are our own placeholder list (real device enumeration isn't ported),
// and this screen's back-target (-> Options hub, screen 1) is our own
// reasonable inference, not something the docs state explicitly.
//
// Shares MenuAssets::background (see MenuAssets.h) rather than a plain
// fill - leaving to the Options hub plays the real optfade2.bik
// transition clip first (../StarLancer/reversing docs Pass 60's call
// map: this exact clip for a sub-screen returning to the main-menu-
// Options context), whose frozen final frame then becomes this screen's
// own backdrop for as long as it's shown.
class SoundOptionsScreen : public MenuScreen {
public:
    SoundOptionsScreen(std::string iniPath, const MenuAssets* assets);

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
    SoundConfig m_config;
    AdjustableList m_rows;
};

} // namespace neoslancer
