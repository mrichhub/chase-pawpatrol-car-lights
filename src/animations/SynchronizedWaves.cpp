#include "SynchronizedWaves.h"
#include <cmath>

namespace {
// Effect-local position of each pixel in its traveling wave. This does not
// constrain other animations or require pixels to share a physical group API.
constexpr std::size_t PositionByPixel[] = {
    3, 2, 1, 0,  // Blue: 3 -> 2 -> 1 -> 0
    0, 1, 2, 3,  // Red:  4 -> 5 -> 6 -> 7
    0, 1, 2, 3,  // Red:  8 -> 9 -> 10 -> 11
    3, 2, 1, 0   // Blue: 15 -> 14 -> 13 -> 12
};
constexpr auto MappedPixelCount = sizeof(PositionByPixel) / sizeof(PositionByPixel[0]);
constexpr float TwoPi = 6.2831853071795864769f;
}  // namespace

std::uint8_t SynchronizedWaves::brightnessAt(std::size_t position,
                                            std::uint32_t cycleElapsedMs) const {
    if (cycleMs_ == 0 || minBrightness_ == maxBrightness_) {
        return minBrightness_;
    }

    // Pulse starts: 0, 1/6, 2/6, 3/6 of a cycle. Each lasts 1/2 cycle.
    // Adjacent pulses overlap, but the first ends exactly when the last starts.
    // Integer sixths avoid truncating short/odd cycles; 64-bit intermediates
    // also support the full uint32_t cycle range without overflow.
    const auto time = static_cast<std::uint64_t>(cycleElapsedMs) * 6;
    const auto start = static_cast<std::uint64_t>(cycleMs_) * position;
    const auto width = static_cast<std::uint64_t>(cycleMs_) * 3;
    if (time <= start || time - start >= width) {
        return minBrightness_;
    }

    const float phase = static_cast<float>(time - start) / static_cast<float>(width);
    const float pulse = (1.0f - std::cos(TwoPi * phase)) * 0.5f;
    const auto range = static_cast<unsigned>(maxBrightness_ - minBrightness_);
    auto offset = static_cast<unsigned>(pulse * static_cast<float>(range) + 0.5f);
    if (offset > range) {
        offset = range;
    }
    return static_cast<std::uint8_t>(minBrightness_ + offset);
}

bool SynchronizedWaves::render(PixelBuffer& pixels, std::uint32_t elapsedMs) const {
    const auto cycleElapsedMs = cycleMs_ == 0 ? 0 : elapsedMs % cycleMs_;
    std::uint8_t brightness[4];
    for (std::size_t position = 0; position < 4; ++position) {
        brightness[position] = brightnessAt(position, cycleElapsedMs);
    }

    bool changed = false;
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        CRGB color = CRGB::Black;
        if (i < MappedPixelCount) {
            const auto intensity = brightness[PositionByPixel[i]];
            // Pure red/blue scaling is exactly the selected channel intensity.
            // FastLED applies GlobalBrightness once, later in main.cpp.
            color = (i < 4 || i >= 12) ? CRGB(0, 0, intensity) : CRGB(intensity, 0, 0);
        }
        changed = pixels.set(i, color) || changed;
    }
    return changed;
}
