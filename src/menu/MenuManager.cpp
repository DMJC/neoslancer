#include "neoslancer/menu/MenuManager.h"

#include <cstdio>

namespace neoslancer {

MenuManager::MenuManager(UIRenderer& renderer, Font& font) : m_renderer(renderer), m_font(font) {}

void MenuManager::registerScreen(int screenId, std::unique_ptr<MenuScreen> screen) {
    m_screens[screenId] = std::move(screen);
}

void MenuManager::goTo(int screenId) {
    const auto it = m_screens.find(screenId);
    if (it == m_screens.end()) {
        // Matches RunMenuScreenLoop's documented "other -> no-op, returns 3"
        // fallback for an unregistered screen ID.
        std::fprintf(stderr, "neoslancer: no menu screen registered for ID %d\n", screenId);
        return;
    }
    m_currentId = screenId;
    it->second->onEnter(*this);
}

void MenuManager::handleEvent(const SDL_Event& event) {
    const auto it = m_screens.find(m_currentId);
    if (it != m_screens.end()) {
        it->second->handleEvent(event, *this);
    }
}

bool MenuManager::update(float deltaSeconds) {
    const auto it = m_screens.find(m_currentId);
    if (it == m_screens.end()) {
        return false;
    }
    return it->second->update(deltaSeconds, *this);
}

void MenuManager::render(int windowWidth, int windowHeight) {
    const auto it = m_screens.find(m_currentId);
    if (it != m_screens.end()) {
        it->second->render(m_renderer, m_font, windowWidth, windowHeight);
    }
}

} // namespace neoslancer
