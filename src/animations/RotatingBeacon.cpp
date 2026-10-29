#include "RotatingBeacon.h"
#include "EffectMath.h"
#include <cmath>

bool RotatingBeacon::render(PixelBuffer& pixels, std::uint32_t elapsedMs) const {
    const float head = effect_math::phase(elapsedMs, rotationMs_) * 4.0f;
    std::uint8_t brightness[4];
    for (std::size_t p = 0; p < 4; ++p) {
        const float linearDistance = std::abs(static_cast<float>(p) - head);
        const float distance = linearDistance < 2.0f ? linearDistance
                                                    : 4.0f - linearDistance;
        // Compact circular cosine lobe: full at the head, 25% at its two
        // neighboring pixel centers, zero on the opposite side of the circle.
        const float glow = rotationMs_ == 0 || distance >= 1.5f ? 0.0f
            : 0.5f * (1.0f + std::cos(effect_math::Pi * distance / 1.5f));
        brightness[p] = effect_math::intensity(minBrightness_, maxBrightness_, glow);
    }

    bool changed = false;
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        changed = pixels.set(i, effect_math::fixedColor(i, brightness[i % 4])) || changed;
    }
    return changed;
}
