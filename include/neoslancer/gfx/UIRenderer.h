#pragma once

#include "neoslancer/gfx/Font.h"
#include "neoslancer/gfx/Shader.h"

#include <GL/glew.h>

#include <string>
#include <unordered_map>

namespace neoslancer {

struct Color {
    float r, g, b, a;
};

// A small immediate-mode-style 2D renderer (rects + cached text quads) for
// menu UI, in pixel coordinates with a top-left origin. Not a general
// engine renderer - just enough for menu screens until the real .spr/.fnt
// asset formats are decoded and a proper sprite renderer replaces this.
class UIRenderer {
public:
    bool init();
    void shutdown();

    void resize(int width, int height);

    void drawRect(float x, float y, float w, float h, Color color) const;

    // Draws an arbitrary caller-owned GL texture (e.g. a decoded video
    // frame) as one quad. tint multiplies the sampled color.
    void drawTexture(GLuint texture, float x, float y, float w, float h,
                      Color tint = {1.0f, 1.0f, 1.0f, 1.0f}) const;

    // Renders (and caches) text as a single texture, then draws it as one
    // quad. color tints the (white) glyph texture.
    void drawText(Font& font, const std::string& text, float x, float y, Color color);

    // Text size in pixels, for layout.
    void measureText(Font& font, const std::string& text, int& outWidth, int& outHeight) const;

private:
    struct TextTexture {
        GLuint texture = 0;
        int width = 0;
        int height = 0;
    };

    const TextTexture* getOrCreateTextTexture(Font& font, const std::string& text);
    void drawTexturedRect(float x, float y, float w, float h, GLuint texture, Color color) const;

    Shader m_shader;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLint m_uProjection = -1;
    GLint m_uRect = -1;
    GLint m_uColor = -1;
    GLint m_uUseTexture = -1;

    float m_projection[16] = {};
    int m_width = 0;
    int m_height = 0;

    std::unordered_map<std::string, TextTexture> m_textCache;
};

} // namespace neoslancer
