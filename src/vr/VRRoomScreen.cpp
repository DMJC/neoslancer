#include "neoslancer/vr/VRRoomScreen.h"

#include "neoslancer/menu/MenuManager.h"
#include "neoslancer/menu/MenuScreenIds.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>

namespace neoslancer {

namespace {

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

// Reference resolution the original's hotspot coordinates and .bik assets
// are authored in (confirmed via ffprobe on real files: 640x480).
constexpr int kReferenceWidth = 640;
constexpr int kReferenceHeight = 480;

} // namespace

VRRoomScreen::VRRoomScreen(std::string dataRoot) : m_dataRoot(std::move(dataRoot)) {}

std::string VRRoomScreen::resolveMoviePath(const std::string& fileName) const {
    if (fileName.empty()) {
        return {};
    }

    // Ship-interior movies live under cd1/ or cd2/ in a real install (the
    // original's 2-disc CD-swap system, EnsureCorrectCDMounted) - this
    // port has both discs' data available locally, so just tries both
    // rather than modeling disc-swap prompts.
    const std::string candidates[] = {
        m_dataRoot + "/cd1/" + fileName,
        m_dataRoot + "/cd2/" + fileName,
        m_dataRoot + "/" + fileName,
    };
    for (const std::string& candidate : candidates) {
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
    }

    // Case-insensitive fallback scan, since the real filenames' case
    // doesn't always match the strings embedded in Lancer.exe exactly.
    const std::string wanted = toLower(fileName);
    for (const std::string& dir : {m_dataRoot + "/cd1", m_dataRoot + "/cd2", m_dataRoot}) {
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
            if (toLower(entry.path().filename().string()) == wanted) {
                return entry.path().string();
            }
        }
    }

    return {};
}

void VRRoomScreen::enterNode(uint32_t address) {
    m_currentAddress = address;
    const VRRoomNodeData* node = findVRRoomNode(address);
    std::fprintf(stderr, "neoslancer: VR enterNode(%#x) -> %s\n", address,
                 node ? node->moviePath.c_str() : "(not in graph)");
    if (!node || node->moviePath.empty()) {
        m_video.close();
        return;
    }

    const std::string path = resolveMoviePath(node->moviePath);
    if (path.empty()) {
        std::fprintf(stderr, "neoslancer: ship-interior movie '%s' not found under data root\n",
                     node->moviePath.c_str());
        m_video.close();
        return;
    }
    m_video.open(path);
}

void VRRoomScreen::onEnter(MenuManager& manager) {
    (void)manager;
    m_lateCampaign = true;
    m_pendingExit = false;
    enterNode(kVRLateCampaignEntry);
}

std::vector<VRRoomScreen::HotspotRect> VRRoomScreen::currentHotspots() const {
    std::vector<HotspotRect> rects;
    const VRRoomNodeData* current = findVRRoomNode(m_currentAddress);
    if (!current) {
        return rects;
    }
    for (uint32_t targetAddress : current->targets) {
        const VRRoomNodeData* target = findVRRoomNode(targetAddress);
        if (!target) {
            continue; // not walked into our graph slice yet - not offered as clickable
        }
        rects.push_back({targetAddress, target->hotspotX, target->hotspotY, target->hotspotW, target->hotspotH});
    }
    return rects;
}

void VRRoomScreen::handleEvent(const SDL_Event& event, MenuManager& manager) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        manager.goTo(MenuScreenId::MainMenu);
        return;
    }

    const bool activate = (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) ||
                           (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE);

    if (event.type == SDL_MOUSEMOTION) {
        m_mouseX = event.motion.x;
        m_mouseY = event.motion.y;
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        m_mouseX = event.button.x;
        m_mouseY = event.button.y;
    }

    if (m_hoveredIndex >= 0 && activate) {
        const auto hotspots = currentHotspots();
        if (m_hoveredIndex < static_cast<int>(hotspots.size())) {
            const uint32_t clicked = hotspots[static_cast<size_t>(m_hoveredIndex)].targetAddress;
            bool exitToMenu = false;
            const uint32_t effective = resolveVRRoomTransition(clicked, m_lateCampaign, exitToMenu);
            if (exitToMenu) {
                manager.goTo(MenuScreenId::MainMenu);
            } else {
                enterNode(effective);
            }
        }
    }
}

bool VRRoomScreen::update(float deltaSeconds, MenuManager& manager) {
    (void)manager;
    m_video.update(deltaSeconds);
    return false;
}

void VRRoomScreen::render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) {
    renderer.drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight),
                       Color{0.0f, 0.0f, 0.0f, 1.0f});

    if (m_video.isOpen()) {
        renderer.drawTexture(m_video.texture(), 0, 0, static_cast<float>(windowWidth),
                              static_cast<float>(windowHeight));
    } else {
        const std::string msg = "(no movie loaded for this room)";
        int w = 0, h = 0;
        renderer.measureText(font, msg, w, h);
        renderer.drawText(font, msg, (static_cast<float>(windowWidth) - static_cast<float>(w)) * 0.5f,
                           (static_cast<float>(windowHeight) - static_cast<float>(h)) * 0.5f,
                           Color{0.8f, 0.3f, 0.3f, 1.0f});
    }

    const float scaleX = static_cast<float>(windowWidth) / static_cast<float>(kReferenceWidth);
    const float scaleY = static_cast<float>(windowHeight) / static_cast<float>(kReferenceHeight);

    const auto hotspots = currentHotspots();
    m_hoveredIndex = -1;
    for (size_t i = 0; i < hotspots.size(); ++i) {
        const HotspotRect& r = hotspots[i];
        const float x = static_cast<float>(r.x) * scaleX;
        const float y = static_cast<float>(r.y) * scaleY;
        const float w = static_cast<float>(r.w) * scaleX;
        const float h = static_cast<float>(r.h) * scaleY;

        // Original hit-test is `x < mouseX < x+w`, `y < mouseY < y+h` in
        // 640x480 reference space - mirrored here in window space.
        const bool hovered = static_cast<float>(m_mouseX) > x && static_cast<float>(m_mouseX) < x + w &&
                              static_cast<float>(m_mouseY) > y && static_cast<float>(m_mouseY) < y + h;
        if (hovered) {
            m_hoveredIndex = static_cast<int>(i);
        }

        const Color outline = hovered ? Color{0.9f, 0.85f, 0.2f, 0.35f} : Color{0.4f, 0.7f, 1.0f, 0.15f};
        renderer.drawRect(x, y, w, h, outline);
    }

    const std::string hint = "Click a highlighted area to move - ESC to exit";
    int hintW = 0, hintH = 0;
    renderer.measureText(font, hint, hintW, hintH);
    renderer.drawRect(0, static_cast<float>(windowHeight) - static_cast<float>(hintH) - 16.0f,
                       static_cast<float>(windowWidth), static_cast<float>(hintH) + 16.0f, Color{0, 0, 0, 0.5f});
    renderer.drawText(font, hint, (static_cast<float>(windowWidth) - static_cast<float>(hintW)) * 0.5f,
                       static_cast<float>(windowHeight) - static_cast<float>(hintH) - 8.0f,
                       Color{0.9f, 0.9f, 0.9f, 1.0f});
}

} // namespace neoslancer
