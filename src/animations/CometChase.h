#pragma once

#include "Animation.h"

class CometChase final : public Animation {
public:
    CometChase(std::uint32_t cycleMs, std::uint8_t minBrightness,
               std::uint8_t maxBrightness, std::uint8_t trailLength)
        : cycleMs_(cycleMs), minBrightness_(minBrightness),
          maxBrightness_(maxBrightness), trailLength_(trailLength) {}

    const char* name() const override { return "Comet chase"; }
    bool render(PixelBuffer& pixels, std::uint32_t elapsedMs) const override;

private:
    const std::uint32_t cycleMs_;
    const std::uint8_t minBrightness_;
    const std::uint8_t maxBrightness_;
    const std::uint8_t trailLength_;
};
