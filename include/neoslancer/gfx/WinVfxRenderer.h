#pragma once

#include "neoslancer/WinVfxFont.h"
#include "neoslancer/WinVfxPalette.h"
#include "neoslancer/WinVfxSprite.h"
#include "neoslancer/gfx/UIRenderer.h"

#include <GL/glew.h>

#include <string>
#include <unordered_map>

namespace neoslancer {

// Draws real WinVFX .fnt glyphs and .spr shapes as GL textures, replacing
// the SDL_ttf/colored-rect placeholders elsewhere in gfx/. See
// WinVfxFont.h/WinVfxSprite.h/WinVfxPalette.h for the underlying formats.
//
// Font glyph pixels are a small integer range (this session found exactly
// 0-15 across every glyph in a real handel.fnt) rather than full 0-255
// palette indices - read as a 4-bit anti-aliasing COVERAGE level (0 =
// transparent background, max = fully opaque), not a direct palette
// lookup. This matches VFX_character_draw's own optional remap-table
// blit mode (Pass 39: a caller-supplied lookup table remaps raw glyph
// bytes through a color table when present) - i.e. the glyph bitmap is
// designed to be tinted, not self-colored. Rendered here exactly like an
// SDL_ttf anti-aliased glyph: coverage modulates a caller-chosen solid
// tint color. Confidence 2 on this specific interpretation (a well-
// justified inference, not textually confirmed).
//
// Sprite shape pixels ARE direct WinVfxPalette indices (opaque
// non-transparent pixels only, transparent elsewhere per the RLE
// decode's own opaque mask) - validated this session by colorizing a
// real FRONTEND.SPR shape with a real palette.ccb and getting
// recognizable, correctly-toned artwork, not noise.
class WinVfxRenderer {
public:
    ~WinVfxRenderer();

    void shutdown();

    // Draws text using font's real per-glyph metrics (width used both for
    // rendering and as the advance, matching VFX_character_width).
    void drawText(UIRenderer& renderer, const WinVfxFont& font, const WinVfxPalette& palette,
                   const std::string& text, float x, float y, Color tint);
    void measureText(const WinVfxFont& font, const std::string& text, int& outWidth, int& outHeight) const;

    void drawShape(UIRenderer& renderer, const WinVfxSprite& sprite, size_t shapeIndex, const WinVfxPalette& palette,
                   float x, float y, Color tint = {1.0f, 1.0f, 1.0f, 1.0f});

    // Same, but stretched to an explicit w/h instead of the shape's own
    // native size - for fitting real art into a real, confirmed hotspot
    // rect (e.g. RunMainMenuScreen's 3 button rects) whose dimensions
    // don't exactly match any one decoded shape's own bounding box.
    void drawShapeScaled(UIRenderer& renderer, const WinVfxSprite& sprite, size_t shapeIndex,
                         const WinVfxPalette& palette, float x, float y, float w, float h,
                         Color tint = {1.0f, 1.0f, 1.0f, 1.0f});

    bool shapeSize(const WinVfxSprite& sprite, size_t shapeIndex, int& outWidth, int& outHeight) const;

    // Draws shapeIndex flanking a title's left/right edges, vertically
    // centered against it - the repeated "accent icon beside the title"
    // decoration used by every real-asset menu screen. No-op if the shape
    // can't be sized (e.g. sprite not loaded).
    void drawTitleAccents(UIRenderer& renderer, const WinVfxSprite& sprite, size_t shapeIndex,
                          const WinVfxPalette& palette, float titleX, float titleY, float titleW, float titleH,
                          float gap = 16.0f);

private:
    struct GlTexture {
        GLuint texture = 0;
        int width = 0;
        int height = 0;
    };

    const GlTexture* getOrCreateGlyphTexture(const WinVfxFont& font, uint8_t c);
    const GlTexture* getOrCreateShapeTexture(const WinVfxSprite& sprite, size_t shapeIndex, const WinVfxPalette& palette);

    std::unordered_map<uint64_t, GlTexture> m_glyphCache; // key: (font ptr, char) packed
    std::unordered_map<uint64_t, GlTexture> m_shapeCache; // key: (sprite ptr, shapeIndex) packed
};

} // namespace neoslancer
