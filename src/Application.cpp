#include "neoslancer/Application.h"

#include "neoslancer/menu/ControlsOptionsScreen.h"
#include "neoslancer/menu/MainMenuScreen.h"
#include "neoslancer/menu/MenuScreenIds.h"
#include "neoslancer/menu/NetworkDisconnectScreen.h"
#include "neoslancer/menu/OptionsMenuScreen.h"
#include "neoslancer/menu/PlaceholderScreen.h"
#include "neoslancer/menu/SoundOptionsScreen.h"
#include "neoslancer/menu/VideoOptionsScreen.h"

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include <cstdio>

namespace neoslancer {

namespace {

void registerMenuScreens(MenuManager& manager, const std::string& iniPath) {
    using namespace MenuScreenId;

    manager.registerScreen(MainMenu, std::make_unique<MainMenuScreen>());
    manager.registerScreen(OptionsMenu, std::make_unique<OptionsMenuScreen>());
    manager.registerScreen(SoundOptions, std::make_unique<SoundOptionsScreen>(iniPath));
    manager.registerScreen(VideoOptions, std::make_unique<VideoOptionsScreen>(iniPath));
    manager.registerScreen(ControlsOptions, std::make_unique<ControlsOptionsScreen>(iniPath));
    manager.registerScreen(NetworkDisconnect, std::make_unique<NetworkDisconnectScreen>());

    // Real RunMenuScreenLoop handlers this port hasn't built real behavior
    // for yet (confidence_db.md has what each one actually does).
    manager.registerScreen(NewGameSetup,
                            std::make_unique<PlaceholderScreen>(
                                "NEW GAME", "Campaign/pilot setup", MainMenu));
    manager.registerScreen(MissionBriefing,
                            std::make_unique<PlaceholderScreen>(
                                "MISSION BRIEFING", "Briefing movie + ship-interior handoff", MainMenu));
    manager.registerScreen(SaveGame,
                            std::make_unique<PlaceholderScreen>("SAVE GAME", "Save-slot browser", OptionsMenu));
    manager.registerScreen(LoadGame,
                            std::make_unique<PlaceholderScreen>("LOAD GAME", "Save-slot browser", OptionsMenu));
    manager.registerScreen(SaveGameBrowser,
                            std::make_unique<PlaceholderScreen>(
                                "SAVE GAME BROWSER", "Save-file scanner", MainMenu));
    manager.registerScreen(MultiplayerSetup,
                            std::make_unique<PlaceholderScreen>(
                                "MULTIPLAYER", "Direct/Zone.com/Host/Join connection setup", MainMenu));
    manager.registerScreen(MultiplayerLobbyHost,
                            std::make_unique<PlaceholderScreen>(
                                "HOST GAME", "Multiplayer lobby (host)", MultiplayerSetup));
    manager.registerScreen(MultiplayerLobbyJoin,
                            std::make_unique<PlaceholderScreen>(
                                "JOIN GAME", "Multiplayer lobby (join)", MultiplayerSetup));

    manager.goTo(MainMenu);
}

} // namespace

Application::~Application() {
    shutdown();
}

bool Application::init(const WindowConfig& config, const std::string& iniPath) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    if (!m_window.create(config)) {
        SDL_Quit();
        return false;
    }

    if (TTF_Init() != 0) {
        std::fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        m_window.destroy();
        SDL_Quit();
        return false;
    }
    m_font.loadSystemFont(20); // non-fatal if it fails - text just won't render

    glClearColor(0.02f, 0.02f, 0.06f, 1.0f);
    // No depth testing: every UI quad is drawn at the same z=0, and with
    // GL_DEPTH_TEST on the default GL_LESS test rejects every draw after
    // the first at an already-occupied depth (menu render order is what
    // establishes front-to-back, not depth). 3D scene rendering, when it
    // exists, will need to enable this itself.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!m_uiRenderer.init()) {
        TTF_Quit();
        m_window.destroy();
        SDL_Quit();
        return false;
    }
    m_uiRenderer.resize(m_window.width(), m_window.height());

    m_menuManager = std::make_unique<MenuManager>(m_uiRenderer, m_font);
    registerMenuScreens(*m_menuManager, iniPath);

    m_initialized = true;
    return true;
}

void Application::run() {
    if (!m_initialized) {
        return;
    }

    m_running = true;
    uint64_t previousTicks = SDL_GetPerformanceCounter();
    const uint64_t frequency = SDL_GetPerformanceFrequency();

    while (m_running) {
        const uint64_t currentTicks = SDL_GetPerformanceCounter();
        const float deltaSeconds =
            static_cast<float>(currentTicks - previousTicks) / static_cast<float>(frequency);
        previousTicks = currentTicks;

        handleEvents();
        update(deltaSeconds);
        render();
    }
}

void Application::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                m_running = false;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                    event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    m_window.onResize(event.window.data1, event.window.data2);
                    m_uiRenderer.resize(event.window.data1, event.window.data2);
                }
                break;
            default:
                m_menuManager->handleEvent(event);
                break;
        }
    }

    if (m_menuManager->quitRequested()) {
        m_running = false;
    }
}

void Application::update(float deltaSeconds) {
    m_menuManager->update(deltaSeconds);
}

void Application::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_menuManager->render(m_window.width(), m_window.height());

    m_window.swap();
}

void Application::shutdown() {
    if (!m_initialized) {
        return;
    }
    m_menuManager.reset();
    m_uiRenderer.shutdown();
    m_font.close();
    TTF_Quit();
    m_window.destroy();
    SDL_Quit();
    m_initialized = false;
}

} // namespace neoslancer
