#include "neoslancer/menu/OptionsMenuScreen.h"

#include "neoslancer/menu/MenuManager.h"
#include "neoslancer/menu/MenuScreenIds.h"

namespace neoslancer {

void OptionsMenuScreen::onEnter(MenuManager& manager) {
    setTitle("OPTIONS");
    buttons().setButtons({
        {"SOUND", [&manager]() { manager.goTo(MenuScreenId::SoundOptions); }},
        {"VIDEO", [&manager]() { manager.goTo(MenuScreenId::VideoOptions); }},
        {"CONTROLS", [&manager]() { manager.goTo(MenuScreenId::ControlsOptions); }},
        {"BACK", [&manager]() { manager.goTo(MenuScreenId::MainMenu); }},
    });
    onEscape = [&manager]() { manager.goTo(MenuScreenId::MainMenu); };
}

} // namespace neoslancer
