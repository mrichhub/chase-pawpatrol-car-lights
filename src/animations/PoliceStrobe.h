#pragma once

#include "Animation.h"

class PoliceStrobe final : public Animation {
public:
    PoliceStrobe(std::uint32_t colorPhaseMs, std::uint32_t onMs,
                 std::uint32_t offMs)
        : colorPhaseMs_(colorPhaseMs), onMs_(onMs), offMs_(offMs) {}

    const char* name() const override { return "Alternating police strobes"; }
    bool render(PixelBuffer& pixels, std::uint32_t elapsedMs) const override;

private:
    const std::uint32_t colorPhaseMs_;
    const std::uint32_t onMs_;
    const std::uint32_t offMs_;
};
