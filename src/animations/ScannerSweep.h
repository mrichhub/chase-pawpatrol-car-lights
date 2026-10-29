#pragma once

#include "Animation.h"

class ScannerSweep final : public Animation {
public:
    ScannerSweep(std::uint32_t roundTripMs, std::uint8_t minBrightness,
                 std::uint8_t maxBrightness, std::uint8_t trailStrength)
        : roundTripMs_(roundTripMs), minBrightness_(minBrightness),
          maxBrightness_(maxBrightness), trailStrength_(trailStrength) {}

    const char* name() const override { return "Scanner sweep"; }
    bool render(PixelBuffer& pixels, std::uint32_t elapsedMs) const override;

private:
    const std::uint32_t roundTripMs_;
    const std::uint8_t minBrightness_;
    const std::uint8_t maxBrightness_;
    const std::uint8_t trailStrength_;
};
