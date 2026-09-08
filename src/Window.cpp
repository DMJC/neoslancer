#include "neoslancer/Window.h"

#include <GL/glew.h>
#include <SDL_opengl.h>

#include <cstdio>

namespace neoslancer {

Window::~Window() {
    destroy();
}

bool Window::create(const WindowConfig& config) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    if (config.fullscreen) {
        // Borderless-at-desktop-resolution rather than an exclusive mode
        // switch to Xres/Yres: most modern Linux compositors (Wayland
        // especially) don't honor arbitrary exclusive-fullscreen mode
        // switches, so this is the reliable choice even though it means
        // [Device] Xres/Yres only take effect in windowed mode.
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    m_window = SDL_CreateWindow(
        config.title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        config.width, config.height,
        flags);

    if (!m_window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        std::fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        destroy();
        return false;
    }

    SDL_GL_SetSwapInterval(config.vsync ? 1 : 0);

    // Best-effort: many modern display servers (Wayland, most compositors)
    // don't support legacy gamma ramps at all, so failure here is silently
    // ignored rather than treated as fatal.
    SDL_SetWindowBrightness(m_window, static_cast<float>(config.gamma) / 100.0f);

    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::fprintf(stderr, "glewInit failed: %s\n", glewGetErrorString(glewStatus));
        destroy();
        return false;
    }
    // glewInit() can leave a spurious GL_INVALID_ENUM on the error stack; clear it.
    glGetError();

    m_width = config.width;
    m_height = config.height;
    return true;
}

void Window::destroy() {
    if (m_glContext) {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
}

void Window::swap() const {
    SDL_GL_SwapWindow(m_window);
}

void Window::onResize(int width, int height) {
    m_width = width;
    m_height = height;
    glViewport(0, 0, width, height);
}

} // namespace neoslancer
