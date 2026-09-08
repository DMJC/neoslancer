#include "neoslancer/menu/PlaceholderScreen.h"

#include "neoslancer/menu/MenuManager.h"

namespace neoslancer {

PlaceholderScreen::PlaceholderScreen(std::string title, std::string description, int backScreenId,
                                      const MenuAssets* assets)
    : SimpleMenuScreen(assets),
      m_titleText(std::move(title)),
      m_descriptionText(std::move(description)),
      m_backScreenId(backScreenId) {}

void PlaceholderScreen::onEnter(MenuManager& manager) {
    setTitle(m_titleText);
    setBody(m_descriptionText + " (not implemented yet)");
    const int backId = m_backScreenId;
    buttons().setButtons({
        {"BACK", [&manager, backId]() { manager.goTo(backId); }},
    });
    onEscape = [&manager, backId]() { manager.goTo(backId); };
}

} // namespace neoslancer
