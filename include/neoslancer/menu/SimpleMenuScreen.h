#pragma once

#include "neoslancer/menu/ButtonList.h"
#include "neoslancer/menu/MenuAssets.h"
#include "neoslancer/menu/MenuScreen.h"

#include <functional>
#include <string>

namespace neoslancer {

// Shared "title + vertical button list, optional body text" layout, used
// by every screen here that doesn't need custom widgets (options hub,
// and every not-yet-implemented placeholder).
//
// Renders with the real WinVFX handel.fnt bitmap font (see MenuAssets.h)
// when `assets` is non-null and loaded; falls back to the SDL_ttf
// placeholder font otherwise, same pattern as MainMenuScreen.
class SimpleMenuScreen : public MenuScreen {
public:
    explicit SimpleMenuScreen(const MenuAssets* assets) : m_assets(assets) {}

    void handleEvent(const SDL_Event& event, MenuManager& manager) override;
    void render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) override;

protected:
    void setTitle(std::string title) { m_title = std::move(title); }
    void setBody(std::string body) { m_body = std::move(body); }
    ButtonList& buttons() { return m_buttons; }

    // Escape triggers this if set (typically "go back"); otherwise Escape
    // does nothing on this screen.
    std::function<void()> onEscape;

private:
    void renderWithWinVfx(UIRenderer& renderer, int windowWidth, int windowHeight);
    void renderFallback(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight);

    const MenuAssets* m_assets;
    std::string m_title;
    std::string m_body;
    ButtonList m_buttons;
};

} // namespace neoslancer
