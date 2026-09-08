#pragma once

#include <SDL_ttf.h>

namespace neoslancer {

// Placeholder text rendering until the original .fnt bitmap-font format is
// decoded and ported (see reversing docs on RunControlsOptionsScreen etc.
// for confirmed .fnt usage - the format itself isn't decoded yet). Uses
// SDL_ttf against whatever system font can be found, so menu text is at
// least legible in the meantime.
class Font {
public:
    Font() = default;
    ~Font();

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    // Tries a short list of common Linux system font paths, in order.
    // Returns false only if none of them could be opened.
    bool loadSystemFont(int pointSize);

    // Must be called before TTF_Quit() - closing a font after TTF_Quit()
    // has torn down FreeType is undefined behavior. Application::shutdown()
    // calls this explicitly rather than relying on ~Font() running (which
    // happens later, when Application's members are destroyed - after its
    // shutdown() has already called TTF_Quit()).
    void close();

    bool isLoaded() const { return m_font != nullptr; }
    TTF_Font* handle() const { return m_font; }

private:
    TTF_Font* m_font = nullptr;
};

} // namespace neoslancer
