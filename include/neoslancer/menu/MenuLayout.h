#pragma once

namespace neoslancer {

// Real StarLancer never ran at any aspect ratio other than 4:3 - every
// hotspot/hover-highlight coordinate recovered from the binary
// (../StarLancer/reversing docs) is in a 640x480 reference space. Shared
// by every menu screen that positions real assets against that space
// (MainMenuScreen, OptionsMenuScreen, ControlsOptionsScreen) so an actual
// window of any aspect ratio pillar/letterboxes a correctly-proportioned
// 4:3 area instead of stretching content off-ratio.
struct ScaledRect {
    float x, y, w, h;
};

// The 4:3 area within windowWidth x windowHeight, centered, as large as
// possible (pillarboxed if the window is wider than 4:3, letterboxed if
// narrower/taller).
ScaledRect menuViewport(int windowWidth, int windowHeight);

// Maps a rect in the game's native 640x480 reference space into that
// viewport.
ScaledRect mapMenuRect(int x, int y, int w, int h, int windowWidth, int windowHeight);

} // namespace neoslancer
