#include "neoslancer/menu/MenuAssets.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace neoslancer {

namespace {
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

// Case-insensitive lookup for one loose file directly inside `dir` - same
// approach IntroScreen/MainMenuScreen each already used before this was
// centralized here.
std::string findInDirectory(const std::string& dir, const std::string& wantedLower) {
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (toLower(entry.path().filename().string()) == wantedLower) {
            return entry.path().string();
        }
    }
    return {};
}

// Loose .bik movies live either directly under dataRoot (the intro/splash
// sequence) or under dataRoot/INTERFACE (the menu-transition fade clips,
// ../StarLancer/reversing docs Pass 60) - not inside RESOURCE.HOG either
// way. Checks both locations rather than assuming which one a given name
// lives in.
std::string findLooseFile(const std::string& dataRoot, const std::string& name) {
    std::string path = dataRoot + "/" + name;
    if (std::filesystem::exists(path)) {
        return path;
    }
    const std::string wanted = toLower(name);
    std::string found = findInDirectory(dataRoot, wanted);
    if (!found.empty()) {
        return found;
    }
    return findInDirectory(dataRoot + "/INTERFACE", wanted);
}
} // namespace

bool loadMenuAssets(const std::string& dataRoot, MenuAssets& out) {
    out.loaded = false;
    out.dataRoot = dataRoot;
    if (!out.archive.open(dataRoot + "/RESOURCE.HOG")) {
        return false;
    }

    const std::vector<uint8_t> fontData = out.archive.read("handel.fnt");
    const std::vector<uint8_t> paletteData = out.archive.read("palette.ccb");

    out.loaded = !fontData.empty() && !paletteData.empty() && parseWinVfxFont(fontData, out.font) &&
                 parseWinVfxPalette(paletteData, out.palette);

    const std::vector<uint8_t> spriteData = out.archive.read("FRONTEND.SPR");
    out.spriteLoaded = !spriteData.empty() && parseWinVfxSprite(spriteData, out.sprite);

    // FRONTEND.SPR carries its own embedded palette (../StarLancer/reversing
    // docs Pass 59/WinVfxSprite.h) - prefer it over the shared palette.ccb
    // wherever it exists, since it's the one this specific file's shapes
    // were actually authored against.
    if (out.spriteLoaded && out.sprite.hasEmbeddedPalette) {
        out.palette = out.sprite.embeddedPalette;
    }

    const std::string backgroundPath = findLooseFile(dataRoot, "SPLASH TO MM.BIK");
    if (!backgroundPath.empty() && out.background.open(backgroundPath)) {
        // Fast-forward to the final frame once - every screen that shares
        // this shows it as a static backdrop, not a replay of the clip.
        out.background.update(9999.0f);
        out.backgroundLoaded = true;
    }

    return out.loaded;
}

bool playMenuTransition(const MenuAssets& assets, const std::string& clipName) {
    const std::string path = findLooseFile(assets.dataRoot, clipName);
    if (path.empty()) {
        return false;
    }
    if (!assets.background.open(path)) {
        return false;
    }
    // Real-time playback (no fast-forward) - the caller's screen must
    // advance it via background.update() from its own update() override,
    // same as IntroScreen does for the logo/splash sequence.
    assets.backgroundLoaded = true;
    return true;
}

} // namespace neoslancer
