#include "neoslancer/menu/MainMenuScreen.h"

#include "neoslancer/menu/MenuManager.h"
#include "neoslancer/menu/MenuScreenIds.h"

namespace neoslancer {

void MainMenuScreen::onEnter(MenuManager& manager) {
    m_buttons.setButtons({
        {"NEW GAME", [&manager]() { manager.goTo(MenuScreenId::NewGameSetup); }},
        {"MULTIPLAYER", [&manager]() { manager.goTo(MenuScreenId::MultiplayerSetup); }},
        {"OPTIONS", [&manager]() { manager.goTo(MenuScreenId::OptionsMenu); }},
        // Not an original main-menu button - the real entry into the VR
        // loop is via RunMissionBriefingScreen (screen 7), which this port
        // hasn't built; this is a direct shortcut to it for now.
        {"SHIP INTERIOR (DEMO)", [&manager]() { manager.goTo(MenuScreenId::MissionBriefing); }},
        {"QUIT", [&manager]() { manager.requestQuitApplication(); }},
    });
}

void MainMenuScreen::handleEvent(const SDL_Event& event, MenuManager& manager) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        manager.requestQuitApplication();
        return;
    }
    m_buttons.handleEvent(event);
}

void MainMenuScreen::render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) {
    renderer.drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight),
                       Color{0.02f, 0.03f, 0.08f, 1.0f});

    const std::string title = "STARLANCER";
    int titleW = 0, titleH = 0;
    renderer.measureText(font, title, titleW, titleH);
    renderer.drawText(font, title, (static_cast<float>(windowWidth) - static_cast<float>(titleW)) * 0.5f,
                       static_cast<float>(windowHeight) * 0.2f, Color{0.75f, 0.85f, 1.0f, 1.0f});

    const float buttonWidth = 340.0f;
    const float buttonHeight = 44.0f;
    const float gap = 12.0f;
    const float x = (static_cast<float>(windowWidth) - buttonWidth) * 0.5f;
    const float y = static_cast<float>(windowHeight) * 0.4f;
    m_buttons.render(renderer, font, x, y, buttonWidth, buttonHeight, gap);
}

} // namespace neoslancer
