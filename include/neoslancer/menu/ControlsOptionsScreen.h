#pragma once

#include "neoslancer/GameConfig.h"
#include "neoslancer/menu/AdjustableList.h"
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
class ControlsOptionsScreen : public MenuScreen {
public:
    explicit ControlsOptionsScreen(std::string iniPath);

    void onEnter(MenuManager& manager) override;
    void handleEvent(const SDL_Event& event, MenuManager& manager) override;
    void render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) override;

private:
    void save();
    void rebuildRows();

    std::string m_iniPath;
    KeyConfigFlags m_flags;
    AdjustableList m_rows;
};

} // namespace neoslancer
