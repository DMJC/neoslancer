#pragma once

#include "neoslancer/menu/SimpleMenuScreen.h"

namespace neoslancer {

// RunOptionsMenuScreen (screen ID 1). Confidence 2: "routes to Sound
// (screen 3), Video (0xf/15), Controls (0x10/16), or back to main menu
// (screen 0)" is a direct decompile fact (confidence_db.md).
class OptionsMenuScreen : public SimpleMenuScreen {
public:
    explicit OptionsMenuScreen(const MenuAssets* assets) : SimpleMenuScreen(assets) {}

    void onEnter(MenuManager& manager) override;
};

} // namespace neoslancer
