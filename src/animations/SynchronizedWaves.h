#pragma once

#include "Animation.h"

class SynchronizedWaves final : public Animation {
public:
    SynchronizedWaves(std::uint8_t minBrightness, std::uint8_t maxBrightness,
                      std::uint32_t cycleMs)
        : minBrightness_(minBrightness),
          maxBrightness_(maxBrightness < minBrightness ? minBrightness : maxBrightness),
          cycleMs_(cycleMs) {}

    const char* name() const override { return "Synchronized brightness waves"; }
    bool render(PixelBuffer& pixels, std::uint32_t elapsedMs) const override;

private:
    std::uint8_t brightnessAt(std::size_t position, std::uint32_t cycleElapsedMs) const;

    const std::uint8_t minBrightness_;
    const std::uint8_t maxBrightness_;
    const std::uint32_t cycleMs_;
};
