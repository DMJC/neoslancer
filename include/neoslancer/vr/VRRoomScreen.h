#pragma once

#include "neoslancer/menu/MenuScreen.h"
#include "neoslancer/video/BinkVideoPlayer.h"
#include "neoslancer/vr/VRRoomGraph.h"

#include <string>

namespace neoslancer {

// RunShipInteriorVRLoop (0x439fb0) - "walk around your ship between
// missions, watch FMV, click hotspots to move to the next room."
//
// Ported: hotspot hit-test against the exact documented rule (a target
// node's own leading hotspotX/Y/W/H, tested `x < mouseX < x+w` and
// `y < mouseY < y+h`, both bounds exclusive), mouse-click OR spacebar
// activation, roomType-driven transitions (1 = exit to menu, 2/5/6/7/9 =
// hard jump to a hub pair overriding the node's own target[]), and real
// video playback of the actual .bik assets (see BinkVideoPlayer.h).
//
// NOT ported: moviePathAlt's one-time arrival clip (always plays
// moviePath instead), roomType 3/4's fixed-rect hover-prop triggers, the
// soundFlag/door-open sound effect, the Esc confirm-dialog (Esc exits
// immediately), and the reverse-playback transition mode. The graph
// itself is also a real but partial slice - see VRRoomGraph.h.
//
// Entry point: the original only reaches this from RunMissionBriefingScreen
// (not itself ported - see MenuScreenId::MissionBriefing), so this screen
// is registered at that same real screen ID and always starts at the
// late-campaign entry node, skipping the briefing movie/speech-tag
// selection this port doesn't have a "current mission index" for yet.
class VRRoomScreen : public MenuScreen {
public:
    explicit VRRoomScreen(std::string dataRoot);

    void onEnter(MenuManager& manager) override;
    void handleEvent(const SDL_Event& event, MenuManager& manager) override;
    bool update(float deltaSeconds, MenuManager& manager) override;
    void render(UIRenderer& renderer, Font& font, int windowWidth, int windowHeight) override;

private:
    void enterNode(uint32_t address);
    std::string resolveMoviePath(const std::string& fileName) const;

    struct HotspotRect {
        uint32_t targetAddress;
        int x, y, w, h;
    };
    std::vector<HotspotRect> currentHotspots() const;

    std::string m_dataRoot;
    uint32_t m_currentAddress = 0;
    bool m_lateCampaign = true;
    BinkVideoPlayer m_video;

    int m_mouseX = 0, m_mouseY = 0;
    int m_hoveredIndex = -1;
    bool m_pendingExit = false;
};

} // namespace neoslancer
