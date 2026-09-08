#pragma once

#include "neoslancer/GameConfig.h"
#include "neoslancer/menu/AdjustableList.h"
#include "neoslancer/menu/MenuAssets.h"
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
class SoundOptionsScreen : public MenuScreen {
public:
    SoundOptionsScreen(std::string iniPath, const MenuAssets* assets);

    void onEnter(MenuManager& manager) override;
    void handleEvent(const SDL_Event& event, MenuManager& manager) override;
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
