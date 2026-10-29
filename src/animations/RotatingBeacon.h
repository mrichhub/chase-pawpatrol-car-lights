#pragma once

#include "Animation.h"

class RotatingBeacon final : public Animation {
public:
    RotatingBeacon(std::uint32_t rotationMs, std::uint8_t minBrightness,
                   std::uint8_t maxBrightness)
        : rotationMs_(rotationMs), minBrightness_(minBrightness),
          maxBrightness_(maxBrightness) {}

    const char* name() const override { return "Rotating beacon"; }
    bool render(PixelBuffer& pixels, std::uint32_t elapsedMs) const override;

private:
    const std::uint32_t rotationMs_;
    const std::uint8_t minBrightness_;
    const std::uint8_t maxBrightness_;
};
