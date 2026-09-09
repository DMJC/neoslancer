#include "neoslancer/menu/MenuLayout.h"

namespace neoslancer {

namespace {
constexpr int kReferenceWidth = 640;
constexpr int kReferenceHeight = 480;
} // namespace

ScaledRect menuViewport(int windowWidth, int windowHeight) {
    const float windowAspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    constexpr float kReferenceAspect = static_cast<float>(kReferenceWidth) / static_cast<float>(kReferenceHeight);
    if (windowAspect > kReferenceAspect) {
        // Window is wider than 4:3 - pillarbox (bars on left/right).
        const float w = static_cast<float>(windowHeight) * kReferenceAspect;
        return {(static_cast<float>(windowWidth) - w) * 0.5f, 0.0f, w, static_cast<float>(windowHeight)};
    }
    // Window is taller/narrower than 4:3 - letterbox (bars on top/bottom).
    const float h = static_cast<float>(windowWidth) / kReferenceAspect;
    return {0.0f, (static_cast<float>(windowHeight) - h) * 0.5f, static_cast<float>(windowWidth), h};
}

ScaledRect mapMenuRect(int x, int y, int w, int h, int windowWidth, int windowHeight) {
    const ScaledRect vp = menuViewport(windowWidth, windowHeight);
    const float scale = vp.w / static_cast<float>(kReferenceWidth);
    return {vp.x + static_cast<float>(x) * scale, vp.y + static_cast<float>(y) * scale, static_cast<float>(w) * scale,
            static_cast<float>(h) * scale};
}

} // namespace neoslancer
