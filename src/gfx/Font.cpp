#include "neoslancer/gfx/Font.h"

#include <cstdio>

namespace neoslancer {

namespace {

const char* kCandidatePaths[] = {
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    "/usr/share/fonts/TTF/DejaVuSans.ttf",
};

} // namespace

Font::~Font() {
    close();
}

void Font::close() {
    if (m_font) {
        TTF_CloseFont(m_font);
        m_font = nullptr;
    }
}

bool Font::loadSystemFont(int pointSize) {
    if (m_font) {
        TTF_CloseFont(m_font);
        m_font = nullptr;
    }

    for (const char* path : kCandidatePaths) {
        m_font = TTF_OpenFont(path, pointSize);
        if (m_font) {
            return true;
        }
    }

    std::fprintf(stderr, "neoslancer: no system font found for menu text (tried %zu paths)\n",
                 sizeof(kCandidatePaths) / sizeof(kCandidatePaths[0]));
    return false;
}

} // namespace neoslancer
