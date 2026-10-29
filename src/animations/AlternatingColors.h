#pragma once

#include "Animation.h"

class AlternatingColors final : public Animation {
public:
    explicit AlternatingColors(std::uint32_t phaseMs) : phaseMs_(phaseMs) {}

    const char* name() const override { return "Alternating red and blue"; }
    bool render(PixelBuffer& pixels, std::uint32_t elapsedMs) const override;

private:
    const std::uint32_t phaseMs_;
};
