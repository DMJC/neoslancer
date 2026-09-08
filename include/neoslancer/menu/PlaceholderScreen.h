#pragma once

#include "neoslancer/menu/SimpleMenuScreen.h"

namespace neoslancer {

// An honest "not implemented yet" stand-in for a real RunMenuScreenLoop
// handler this port hasn't built real behavior for (see confidence_db.md
// for what each one actually does in the original - the description
// passed in here is drawn from that, not invented). Only offers a Back
// button to backScreenId.
class PlaceholderScreen : public SimpleMenuScreen {
public:
    PlaceholderScreen(std::string title, std::string description, int backScreenId, const MenuAssets* assets);

    void onEnter(MenuManager& manager) override;

private:
    std::string m_titleText;
    std::string m_descriptionText;
    int m_backScreenId;
};

} // namespace neoslancer
