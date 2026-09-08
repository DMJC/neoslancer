#include "neoslancer/gfx/WinVfxRenderer.h"

#include <algorithm>

namespace neoslancer {

namespace {

uint64_t packKey(const void* ptr, uint32_t index) {
    return (reinterpret_cast<uint64_t>(ptr) << 16) ^ static_cast<uint64_t>(index);
}

GLuint uploadRgba(int width, int height, const std::vector<uint8_t>& rgba) {
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

} // namespace

WinVfxRenderer::~WinVfxRenderer() {
    shutdown();
}

void WinVfxRenderer::shutdown() {
    for (auto& [key, tex] : m_glyphCache) {
        if (tex.texture != 0) glDeleteTextures(1, &tex.texture);
    }
    for (auto& [key, tex] : m_shapeCache) {
        if (tex.texture != 0) glDeleteTextures(1, &tex.texture);
    }
    m_glyphCache.clear();
    m_shapeCache.clear();
}

const WinVfxRenderer::GlTexture* WinVfxRenderer::getOrCreateGlyphTexture(const WinVfxFont& font, uint8_t c) {
    const uint64_t key = packKey(&font, c);
    const auto it = m_glyphCache.find(key);
    if (it != m_glyphCache.end()) {
        return &it->second;
    }

    const WinVfxGlyph& glyph = font.glyphs[c];
    if (glyph.width == 0 || font.height == 0) {
        return nullptr;
    }

    // Coverage is a small integer range (0-15 observed) rather than
    // 0-255 - normalize per-font against its own real maximum rather
    // than assuming a fixed bit depth.
    uint8_t maxCoverage = 1;
    for (const auto& g : font.glyphs) {
        for (uint8_t p : g.pixels) {
            maxCoverage = std::max(maxCoverage, p);
        }
    }

    const int width = static_cast<int>(glyph.width);
    const int height = static_cast<int>(font.height);
    std::vector<uint8_t> rgba(static_cast<size_t>(width) * height * 4);
    for (int i = 0; i < width * height; ++i) {
        const uint8_t coverage = glyph.pixels[static_cast<size_t>(i)];
        const uint8_t alpha = static_cast<uint8_t>((static_cast<int>(coverage) * 255) / maxCoverage);
        rgba[static_cast<size_t>(i) * 4 + 0] = 255;
        rgba[static_cast<size_t>(i) * 4 + 1] = 255;
        rgba[static_cast<size_t>(i) * 4 + 2] = 255;
        rgba[static_cast<size_t>(i) * 4 + 3] = alpha;
    }

    GlTexture tex;
    tex.width = width;
    tex.height = height;
    tex.texture = uploadRgba(width, height, rgba);
    return &m_glyphCache.emplace(key, tex).first->second;
}

const WinVfxRenderer::GlTexture* WinVfxRenderer::getOrCreateShapeTexture(const WinVfxSprite& sprite,
                                                                          size_t shapeIndex,
                                                                          const WinVfxPalette& palette) {
    const uint64_t key = packKey(&sprite, static_cast<uint32_t>(shapeIndex));
    const auto it = m_shapeCache.find(key);
    if (it != m_shapeCache.end()) {
        return &it->second;
    }

    if (shapeIndex >= sprite.shapes.size()) {
        return nullptr;
    }
    const WinVfxShape& shape = sprite.shapes[shapeIndex];
    if (shape.width == 0 || shape.height == 0) {
        return nullptr;
    }

    std::vector<uint8_t> rgba(static_cast<size_t>(shape.width) * shape.height * 4);
    for (int i = 0; i < shape.width * shape.height; ++i) {
        const size_t o = static_cast<size_t>(i) * 4;
        if (shape.opaque[static_cast<size_t>(i)]) {
            const auto& color = palette.colors[shape.pixels[static_cast<size_t>(i)]];
            rgba[o + 0] = color[0];
            rgba[o + 1] = color[1];
            rgba[o + 2] = color[2];
            rgba[o + 3] = 255;
        } else {
            rgba[o + 0] = rgba[o + 1] = rgba[o + 2] = rgba[o + 3] = 0;
        }
    }

    GlTexture tex;
    tex.width = shape.width;
    tex.height = shape.height;
    tex.texture = uploadRgba(shape.width, shape.height, rgba);
    return &m_shapeCache.emplace(key, tex).first->second;
}

void WinVfxRenderer::drawText(UIRenderer& renderer, const WinVfxFont& font, const WinVfxPalette& palette,
                               const std::string& text, float x, float y, Color tint) {
    (void)palette; // glyph coverage is tinted directly, not palette-colored - see header
    float cursor = x;
    for (unsigned char c : text) {
        const GlTexture* tex = getOrCreateGlyphTexture(font, c);
        if (tex) {
            renderer.drawTexture(tex->texture, cursor, y, static_cast<float>(tex->width),
                                  static_cast<float>(tex->height), tint);
            cursor += static_cast<float>(tex->width);
        } else {
            cursor += static_cast<float>(font.height) * 0.5f; // reasonable fallback advance
        }
    }
}

void WinVfxRenderer::measureText(const WinVfxFont& font, const std::string& text, int& outWidth,
                                  int& outHeight) const {
    outWidth = 0;
    outHeight = static_cast<int>(font.height);
    for (unsigned char c : text) {
        const WinVfxGlyph& glyph = font.glyphs[c];
        outWidth += glyph.width > 0 ? static_cast<int>(glyph.width) : static_cast<int>(font.height) / 2;
    }
}

void WinVfxRenderer::drawShape(UIRenderer& renderer, const WinVfxSprite& sprite, size_t shapeIndex,
                                const WinVfxPalette& palette, float x, float y, Color tint) {
    const GlTexture* tex = getOrCreateShapeTexture(sprite, shapeIndex, palette);
    if (!tex) {
        return;
    }
    renderer.drawTexture(tex->texture, x, y, static_cast<float>(tex->width), static_cast<float>(tex->height), tint);
}

bool WinVfxRenderer::shapeSize(const WinVfxSprite& sprite, size_t shapeIndex, int& outWidth, int& outHeight) const {
    if (shapeIndex >= sprite.shapes.size()) {
        return false;
    }
    outWidth = sprite.shapes[shapeIndex].width;
    outHeight = sprite.shapes[shapeIndex].height;
    return outWidth > 0 && outHeight > 0;
}

} // namespace neoslancer
