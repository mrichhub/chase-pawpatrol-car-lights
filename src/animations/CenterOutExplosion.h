#pragma once

#include "Animation.h"

class CenterOutExplosion final : public Animation {
public:
    CenterOutExplosion(std::uint32_t cycleMs, std::uint8_t minBrightness,
                       std::uint8_t maxBrightness)
        : cycleMs_(cycleMs), minBrightness_(minBrightness),
          maxBrightness_(maxBrightness) {}

    const char* name() const override { return "Center-out explosion"; }
    bool render(PixelBuffer& pixels, std::uint32_t elapsedMs) const override;

private:
    const std::uint32_t cycleMs_;
    const std::uint8_t minBrightness_;
    const std::uint8_t maxBrightness_;
};
