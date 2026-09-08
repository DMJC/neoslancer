#include "neoslancer/Application.h"

#include <GL/glew.h>
#include <SDL.h>

#include <cstdio>

namespace neoslancer {

Application::~Application() {
    shutdown();
}

bool Application::init(const WindowConfig& config) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    if (!m_window.create(config)) {
        SDL_Quit();
        return false;
    }

    glClearColor(0.02f, 0.02f, 0.06f, 1.0f);
    glEnable(GL_DEPTH_TEST);

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
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    m_running = false;
                }
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                    event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    m_window.onResize(event.window.data1, event.window.data2);
                }
                break;
            default:
                break;
        }
    }
}

void Application::update(float /*deltaSeconds*/) {
    // Simulation/game-state update goes here.
}

void Application::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Scene rendering goes here.

    m_window.swap();
}

void Application::shutdown() {
    if (!m_initialized) {
        return;
    }
    m_window.destroy();
    SDL_Quit();
    m_initialized = false;
}

} // namespace neoslancer
