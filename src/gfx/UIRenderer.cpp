#include "neoslancer/gfx/UIRenderer.h"

#include <SDL_surface.h>

namespace neoslancer {

namespace {

const char* kVertexShader = R"(#version 330 core
layout(location = 0) in vec2 aPos;
uniform mat4 uProjection;
uniform vec4 uRect;
out vec2 vUV;
void main() {
    vec2 pos = uRect.xy + aPos * uRect.zw;
    gl_Position = uProjection * vec4(pos, 0.0, 1.0);
    vUV = aPos;
}
)";

const char* kFragmentShader = R"(#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform vec4 uColor;
uniform sampler2D uTexture;
uniform bool uUseTexture;
void main() {
    if (uUseTexture) {
        FragColor = texture(uTexture, vUV) * uColor;
    } else {
        FragColor = uColor;
    }
}
)";

void buildOrtho(float width, float height, float out[16]) {
    for (int i = 0; i < 16; ++i) out[i] = 0.0f;
    out[0] = 2.0f / width;
    out[5] = -2.0f / height;
    out[10] = -1.0f;
    out[12] = -1.0f;
    out[13] = 1.0f;
    out[15] = 1.0f;
}

} // namespace

bool UIRenderer::init() {
    if (!m_shader.compile(kVertexShader, kFragmentShader)) {
        return false;
    }
    m_uProjection = m_shader.uniformLocation("uProjection");
    m_uRect = m_shader.uniformLocation("uRect");
    m_uColor = m_shader.uniformLocation("uColor");
    m_uUseTexture = m_shader.uniformLocation("uUseTexture");

    const float unitQuad[8] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unitQuad), unitQuad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    return true;
}

void UIRenderer::shutdown() {
    for (auto& [text, tex] : m_textCache) {
        if (tex.texture != 0) {
            glDeleteTextures(1, &tex.texture);
        }
    }
    m_textCache.clear();

    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    m_shader.destroy();
}

void UIRenderer::resize(int width, int height) {
    m_width = width;
    m_height = height;
    buildOrtho(static_cast<float>(width), static_cast<float>(height), m_projection);
}

void UIRenderer::drawRect(float x, float y, float w, float h, Color color) const {
    drawTexturedRect(x, y, w, h, 0, color);
}

void UIRenderer::drawTexture(GLuint texture, float x, float y, float w, float h, Color tint) const {
    drawTexturedRect(x, y, w, h, texture, tint);
}

void UIRenderer::drawTexturedRect(float x, float y, float w, float h, GLuint texture, Color color) const {
    m_shader.use();
    glUniformMatrix4fv(m_uProjection, 1, GL_FALSE, m_projection);
    glUniform4f(m_uRect, x, y, w, h);
    glUniform4f(m_uColor, color.r, color.g, color.b, color.a);

    if (texture != 0) {
        glUniform1i(m_uUseTexture, 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
    } else {
        glUniform1i(m_uUseTexture, 0);
    }

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

const UIRenderer::TextTexture* UIRenderer::getOrCreateTextTexture(Font& font, const std::string& text) {
    if (text.empty() || !font.isLoaded()) {
        return nullptr;
    }

    const auto it = m_textCache.find(text);
    if (it != m_textCache.end()) {
        return &it->second;
    }

    const SDL_Color white{255, 255, 255, 255};
    SDL_Surface* rendered = TTF_RenderUTF8_Blended(font.handle(), text.c_str(), white);
    if (!rendered) {
        return nullptr;
    }

    SDL_Surface* converted = SDL_ConvertSurfaceFormat(rendered, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(rendered);
    if (!converted) {
        return nullptr;
    }

    TextTexture entry;
    entry.width = converted->w;
    entry.height = converted->h;

    glGenTextures(1, &entry.texture);
    glBindTexture(GL_TEXTURE_2D, entry.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, converted->w, converted->h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 converted->pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(converted);

    return &m_textCache.emplace(text, entry).first->second;
}

void UIRenderer::drawText(Font& font, const std::string& text, float x, float y, Color color) {
    const TextTexture* tex = getOrCreateTextTexture(font, text);
    if (!tex) {
        return;
    }
    drawTexturedRect(x, y, static_cast<float>(tex->width), static_cast<float>(tex->height), tex->texture, color);
}

void UIRenderer::measureText(Font& font, const std::string& text, int& outWidth, int& outHeight) const {
    outWidth = 0;
    outHeight = 0;
    if (!font.isLoaded() || text.empty()) {
        return;
    }
    TTF_SizeUTF8(font.handle(), text.c_str(), &outWidth, &outHeight);
}

} // namespace neoslancer
