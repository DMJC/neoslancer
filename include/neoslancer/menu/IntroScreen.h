#pragma once

#include "neoslancer/menu/MenuAssets.h"
#include "neoslancer/menu/MenuScreen.h"
#include "neoslancer/video/BinkVideoPlayer.h"

#include <string>
#include <vector>

namespace neoslancer {

// The logo/splash movie sequence WinMain's bootstrap plays before the
// front-end menu ever appears, confirmed by the real files present in the
// game directory and their play order: NEW_NMS.BIK, NEW_DALOGO_FS_UNCMPR.BIK,
// WARTY_.BIK, "SPLASH TO MM.BIK", then the Main Menu. (This ordering wasn't
// independently re-derived from the decompile this session - it's taken
// as given.) Not one of RunMenuScreenLoop's 12 real screens - registered
// at the synthetic MenuScreenId::Intro instead (see that header).
//
// Each clip plays once and, on finishing (or on a click/space/Esc skip),
// advances to the next; after the last one, goes to the Main Menu. Esc
// skips the whole remaining sequence in one press rather than just the
// current clip, matching the usual "skip intro" convention.
class IntroScreen : public MenuScreen {
public:
    IntroScreen(std::string dataRoot, const MenuAssets* assets);

    void onEnter(MenuManager& manager) override;
    void handleEvent(const SDL_Event& event, MenuManager& manager) override;
    bool update(float deltaSeconds, MenuManager& manager) override;
    void render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) override;

private:
    void playCurrent();
    void advance(MenuManager& manager);

    std::string m_dataRoot;
    const MenuAssets* m_assets;
    std::vector<std::string> m_movieNames;
    size_t m_index = 0;
    BinkVideoPlayer m_video;
};

} // namespace neoslancer
