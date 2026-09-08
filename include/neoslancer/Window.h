#pragma once

#include <SDL.h>
#include <cstdint>
#include <string>

namespace neoslancer {

struct WindowConfig {
    std::string title = "neoslancer";
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    bool vsync = true;
};

// Owns the SDL window and its OpenGL context.
class Window {
public:
    Window() = default;
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool create(const WindowConfig& config);
    void destroy();

    void swap() const;

    int width() const { return m_width; }
    int height() const { return m_height; }
    SDL_Window* handle() const { return m_window; }

    void onResize(int width, int height);

private:
    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;
    int m_width = 0;
    int m_height = 0;
};

} // namespace neoslancer
