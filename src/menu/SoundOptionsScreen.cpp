#include "neoslancer/menu/SoundOptionsScreen.h"

#include "neoslancer/menu/MenuManager.h"
#include "neoslancer/menu/MenuScreenIds.h"

#include <algorithm>
#include <array>

namespace neoslancer {

namespace {
constexpr std::array<const char*, 3> kProviderChoices = {"Software", "DirectSound3D", "A3D"};

int providerIndex(const std::string& provider) {
    for (size_t i = 0; i < kProviderChoices.size(); ++i) {
        if (provider == kProviderChoices[i]) {
            return static_cast<int>(i);
        }
    }
    return 0;
}
} // namespace

SoundOptionsScreen::SoundOptionsScreen(std::string iniPath) : m_iniPath(std::move(iniPath)) {}

void SoundOptionsScreen::save() {
    saveSoundConfig(m_iniPath, m_config);
}

void SoundOptionsScreen::rebuildRows() {
    m_rows.setRows({
        {"3D PROVIDER", [this]() { return m_config.provider3D; },
         [this](int dir) {
             int idx = providerIndex(m_config.provider3D);
             idx = (idx + dir + static_cast<int>(kProviderChoices.size())) % static_cast<int>(kProviderChoices.size());
             m_config.provider3D = kProviderChoices[idx];
             save();
         }},
        {"FX VOLUME", [this]() { return std::to_string(m_config.fxVolume); },
         [this](int dir) {
             m_config.fxVolume = std::clamp(m_config.fxVolume + dir * 5, 0, 100);
             save();
         }},
        {"MUSIC VOLUME", [this]() { return std::to_string(m_config.musicVolume); },
         [this](int dir) {
             m_config.musicVolume = std::clamp(m_config.musicVolume + dir * 5, 0, 100);
             save();
         }},
        {"SPEECH VOLUME", [this]() { return std::to_string(m_config.speechVolume); },
         [this](int dir) {
             m_config.speechVolume = std::clamp(m_config.speechVolume + dir * 5, 0, 100);
             save();
         }},
        {"MASTER VOLUME", [this]() { return std::to_string(m_config.masterVolume); },
         [this](int dir) {
             m_config.masterVolume = std::clamp(m_config.masterVolume + dir * 5, 0, 100);
             save();
         }},
    });
}

void SoundOptionsScreen::onEnter(MenuManager& manager) {
    (void)manager;
    m_config = loadSoundConfig(m_iniPath);
    rebuildRows();
}

void SoundOptionsScreen::handleEvent(const SDL_Event& event, MenuManager& manager) {
    if (event.type == SDL_KEYDOWN &&
        (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_BACKSPACE)) {
        manager.goTo(MenuScreenId::OptionsMenu);
        return;
    }
    m_rows.handleEvent(event);
}

void SoundOptionsScreen::render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) {
    renderer.drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight),
                       Color{0.02f, 0.03f, 0.08f, 1.0f});

    const std::string title = "SOUND OPTIONS";
    int titleW = 0, titleH = 0;
    renderer.measureText(font, title, titleW, titleH);
    float cursorY = static_cast<float>(windowHeight) * 0.12f;
    renderer.drawText(font, title, (static_cast<float>(windowWidth) - static_cast<float>(titleW)) * 0.5f, cursorY,
                       Color{0.75f, 0.85f, 1.0f, 1.0f});
    cursorY += static_cast<float>(titleH) + 24.0f;

    const float width = 360.0f;
    const float x = (static_cast<float>(windowWidth) - width) * 0.5f;
    m_rows.render(renderer, font, x, cursorY, width, 40.0f, 10.0f);

    const std::string hint = "LEFT/RIGHT to adjust, ESC to go back";
    int hintW = 0, hintH = 0;
    renderer.measureText(font, hint, hintW, hintH);
    renderer.drawText(font, hint, (static_cast<float>(windowWidth) - static_cast<float>(hintW)) * 0.5f,
                       static_cast<float>(windowHeight) - static_cast<float>(hintH) - 24.0f,
                       Color{0.5f, 0.5f, 0.55f, 1.0f});
}

} // namespace neoslancer
