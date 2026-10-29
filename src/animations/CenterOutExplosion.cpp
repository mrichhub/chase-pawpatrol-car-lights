#include "CenterOutExplosion.h"
#include "EffectMath.h"

bool CenterOutExplosion::render(PixelBuffer& pixels, std::uint32_t elapsedMs) const {
    const float phase = effect_math::phase(elapsedMs, cycleMs_) * 4.0f;
    float center = 0.0f;
    float outer = 0.0f;
    if (phase < 1.0f) {
        center = effect_math::smoothstep(phase);
    } else if (phase < 2.0f) {
        outer = effect_math::smoothstep(phase - 1.0f);
        center = 1.0f - outer;
    } else if (phase < 3.0f) {
        center = effect_math::smoothstep(phase - 2.0f);
        outer = 1.0f - center;
    } else {
        center = 1.0f - effect_math::smoothstep(phase - 3.0f);
    }

    bool changed = false;
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        const bool isCenter = i % 4 == 1 || i % 4 == 2;
        const auto intensity = effect_math::intensity(minBrightness_, maxBrightness_,
                                                       isCenter ? center : outer);
        changed = pixels.set(i, effect_math::fixedColor(i, intensity)) || changed;
    }
    return changed;
}
