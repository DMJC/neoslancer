#include "neoslancer/menu/IntroScreen.h"

#include "neoslancer/menu/MenuManager.h"
#include "neoslancer/menu/MenuScreenIds.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>

namespace neoslancer {

namespace {
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}
} // namespace

IntroScreen::IntroScreen(std::string dataRoot)
    : m_dataRoot(std::move(dataRoot)),
      m_movieNames{"NEW_NMS.BIK", "NEW_DALOGO_FS_UNCMPR.BIK", "WARTY_.BIK", "SPLASH TO MM.BIK"} {}

void IntroScreen::onEnter(MenuManager& manager) {
    (void)manager;
    m_index = 0;
    playCurrent();
}

void IntroScreen::playCurrent() {
    if (m_index >= m_movieNames.size()) {
        return;
    }
    const std::string& name = m_movieNames[m_index];

    std::string path = m_dataRoot + "/" + name;
    if (!std::filesystem::exists(path)) {
        const std::string wanted = toLower(name);
        std::error_code ec;
        path.clear();
        for (const auto& entry : std::filesystem::directory_iterator(m_dataRoot, ec)) {
            if (toLower(entry.path().filename().string()) == wanted) {
                path = entry.path().string();
                break;
            }
        }
    }

    if (path.empty()) {
        std::fprintf(stderr, "neoslancer: intro movie '%s' not found under data root\n", name.c_str());
        m_video.close();
        return;
    }
    m_video.open(path);
}

void IntroScreen::advance(MenuManager& manager) {
    ++m_index;
    if (m_index >= m_movieNames.size()) {
        manager.goTo(MenuScreenId::MainMenu);
        return;
    }
    playCurrent();
}

void IntroScreen::handleEvent(const SDL_Event& event, MenuManager& manager) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        manager.goTo(MenuScreenId::MainMenu);
        return;
    }
    const bool skipOne = (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) ||
                         (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) ||
                         (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN);
    if (skipOne) {
        advance(manager);
    }
}

bool IntroScreen::update(float deltaSeconds, MenuManager& manager) {
    m_video.update(deltaSeconds);
    if (!m_video.isOpen() || m_video.isFinished()) {
        advance(manager);
    }
    return false;
}

void IntroScreen::render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) {
    renderer.drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight),
                       Color{0.0f, 0.0f, 0.0f, 1.0f});

    if (m_video.isOpen()) {
        renderer.drawTexture(m_video.texture(), 0, 0, static_cast<float>(windowWidth),
                              static_cast<float>(windowHeight));
    }

    const std::string hint = "Click, Space, or Enter to skip - ESC to skip intro";
    int hintW = 0, hintH = 0;
    renderer.measureText(font, hint, hintW, hintH);
    renderer.drawText(font, hint, (static_cast<float>(windowWidth) - static_cast<float>(hintW)) * 0.5f,
                       static_cast<float>(windowHeight) - static_cast<float>(hintH) - 16.0f,
                       Color{0.7f, 0.7f, 0.7f, 0.8f});
}

} // namespace neoslancer
