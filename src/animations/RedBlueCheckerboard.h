#pragma once

#include "Animation.h"

class RedBlueCheckerboard final : public Animation {
public:
    RedBlueCheckerboard(std::uint32_t phaseMs, std::uint8_t brightness)
        : phaseMs_(phaseMs), brightness_(brightness) {}

    const char* name() const override { return "Red/blue checkerboard"; }
    bool render(PixelBuffer& pixels, std::uint32_t elapsedMs) const override;

private:
    const std::uint32_t phaseMs_;
    const std::uint8_t brightness_;
};
