#pragma once

#include "neoslancer/GameConfig.h"
#include "neoslancer/menu/AdjustableList.h"
#include "neoslancer/menu/MenuAssets.h"
#include "neoslancer/menu/MenuScreen.h"

#include <string>

namespace neoslancer {

// RunVideoOptionsScreen (screen ID 15). Confidence 3: confirmed via
// [Device] gamma/Transitions ini keys, and cycles texture/geometry
// detail, lightmaps, and 3D provider (confidence_db.md). This screen
// really does read/write starlancer.ini's [Device] section through
// GameConfig - not a placeholder. What's NOT ported: the original's
// resolution/display-mode LIST comes from a dmodes.bin device-capability
// enumeration this project hasn't built yet, so resolution here is
// display-only, and a gamma/detail change doesn't live-apply to the open
// window (it takes effect on next launch, since Window doesn't currently
// expose a "reconfigure" call). Back-target (-> Options hub, screen 1) is
// our own inference, as with SoundOptionsScreen.
class VideoOptionsScreen : public MenuScreen {
public:
    VideoOptionsScreen(std::string iniPath, const MenuAssets* assets);

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
    DeviceConfig m_config;
    AdjustableList m_rows;
};

} // namespace neoslancer
