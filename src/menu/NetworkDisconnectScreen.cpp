#include "neoslancer/menu/NetworkDisconnectScreen.h"

#include "neoslancer/menu/MenuManager.h"
#include "neoslancer/menu/MenuScreenIds.h"

namespace neoslancer {

void NetworkDisconnectScreen::onEnter(MenuManager& manager) {
    (void)manager;
    m_elapsedSeconds = 0.0f;
}

void NetworkDisconnectScreen::handleEvent(const SDL_Event&, MenuManager&) {
    // No input handled - this screen only exits on its own timer, matching
    // the original's unconditional Sleep(1000).
}

bool NetworkDisconnectScreen::update(float deltaSeconds, MenuManager& manager) {
    m_elapsedSeconds += deltaSeconds;
    if (m_elapsedSeconds >= 1.0f) {
        manager.goTo(MenuScreenId::SoundOptions);
    }
    return false;
}

void NetworkDisconnectScreen::render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) {
    renderer.drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight),
                       Color{0.02f, 0.03f, 0.08f, 1.0f});

    const std::string text = "Disconnecting...";
    const Color tint{0.8f, 0.85f, 0.9f, 1.0f};
    int w = 0, h = 0;
    if (m_assets && m_assets->loaded) {
        m_assets->renderer.measureText(m_assets->font, text, w, h);
        m_assets->renderer.drawText(renderer, m_assets->font, m_assets->palette, text,
                                     (static_cast<float>(windowWidth) - static_cast<float>(w)) * 0.5f,
                                     (static_cast<float>(windowHeight) - static_cast<float>(h)) * 0.5f, tint);
    } else {
        renderer.measureText(font, text, w, h);
        renderer.drawText(font, text, (static_cast<float>(windowWidth) - static_cast<float>(w)) * 0.5f,
                           (static_cast<float>(windowHeight) - static_cast<float>(h)) * 0.5f, tint);
    }
}

} // namespace neoslancer
