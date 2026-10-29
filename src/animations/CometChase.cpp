#include "CometChase.h"
#include "EffectMath.h"
#include <cmath>

namespace {
constexpr std::size_t PositionByPixel[] = {
    3, 2, 1, 0,  // 3 -> 2 -> 1 -> 0
    0, 1, 2, 3,  // 4 -> 5 -> 6 -> 7
    0, 1, 2, 3,  // 8 -> 9 -> 10 -> 11
    3, 2, 1, 0   // 15 -> 14 -> 13 -> 12
};
}  // namespace

bool CometChase::render(PixelBuffer& pixels, std::uint32_t elapsedMs) const {
    // A zero trail setting selects the shortest smooth (one-pixel) tail.
    const float tail = trailLength_ == 0 ? 1.0f : static_cast<float>(trailLength_);
    // Enter from -1; leave past pixel 3 and the whole tail, then rest for one
    // virtual pixel's travel time. No circular head or tail wrapping occurs.
    const float head = -1.0f + (5.0f + tail) * effect_math::phase(elapsedMs, cycleMs_);
    std::uint8_t brightness[4];
    for (std::size_t p = 0; p < 4; ++p) {
        const float distance = head - static_cast<float>(p);
        float glow = 0.0f;
        if (cycleMs_ != 0 && distance > -1.0f && distance < 0.0f) {
            glow = effect_math::smoothstep(1.0f + distance);  // Short leading edge.
        } else if (cycleMs_ != 0 && distance >= 0.0f && distance < tail) {
            const float age = distance / tail;
            // Exponential trail with a smooth finite cutoff. It reaches zero
            // before the next comet enters, even at a nonzero brightness floor.
            glow = std::exp(-2.0f * age) * (1.0f - effect_math::smoothstep(age));
        }
        brightness[p] = effect_math::intensity(minBrightness_, maxBrightness_, glow);
    }

    bool changed = false;
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        const auto intensity = i < 16 ? brightness[PositionByPixel[i]] : 0;
        changed = pixels.set(i, effect_math::fixedColor(i, intensity)) || changed;
    }
    return changed;
}
