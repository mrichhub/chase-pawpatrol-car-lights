#include "ScannerSweep.h"
#include "EffectMath.h"
#include <cmath>

namespace {
float headPosition(float phase) {
    // Cosine motion slows to a stop at each endpoint before reversing.
    return 1.5f * (1.0f - std::cos(effect_math::TwoPi * phase));
}

float headKernel(float pixel, float head) {
    return effect_math::smoothstep(1.0f - std::abs(pixel - head));
}
}  // namespace

bool ScannerSweep::render(PixelBuffer& pixels, std::uint32_t elapsedMs) const {
    const float phase = effect_math::phase(elapsedMs, roundTripMs_);
    float brightness[4] = {};
    // Three past positions form a deterministic trail, without stored frames.
    // Strength maps 0..255 to a first echo weight 0..0.45; later echoes use its
    // powers. This keeps the nearest head pixel at least as bright as the trail.
    const float gain = 0.45f * trailStrength_ / 255.0f;
    for (std::size_t p = 0; p < 4; ++p) {
        brightness[p] = roundTripMs_ == 0 ? 0.0f
            : headKernel(static_cast<float>(p), headPosition(phase));
        float weight = gain;
        for (unsigned echo = 1; echo <= 3 && roundTripMs_ != 0; ++echo) {
            const float past = headPosition(phase - static_cast<float>(echo) / 6.0f);
            const float glow = weight * headKernel(static_cast<float>(p), past);
            if (glow > brightness[p]) {
                brightness[p] = glow;
            }
            weight *= gain;
        }
    }

    bool changed = false;
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        const auto intensity = effect_math::intensity(minBrightness_, maxBrightness_,
                                                       brightness[i % 4]);
        changed = pixels.set(i, effect_math::fixedColor(i, intensity)) || changed;
    }
    return changed;
}
