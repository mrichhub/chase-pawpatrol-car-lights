#pragma once

#include "Animation.h"

class PoliceTripleFlash final : public Animation {
public:
    PoliceTripleFlash(std::uint32_t onMs, std::uint32_t offMs,
                      std::uint32_t pauseMs, std::uint8_t brightness)
        : onMs_(onMs), offMs_(offMs), pauseMs_(pauseMs), brightness_(brightness) {}

    const char* name() const override { return "Police triple flash"; }
    bool render(PixelBuffer& pixels, std::uint32_t elapsedMs) const override;

private:
    const std::uint32_t onMs_;
    const std::uint32_t offMs_;
    const std::uint32_t pauseMs_;
    const std::uint8_t brightness_;
};
