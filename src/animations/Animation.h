#pragma once

#include "engine/PixelBuffer.h"
#include <cstdint>

class Animation {
public:
    virtual ~Animation() = default;
    virtual const char* name() const = 0;

    // elapsedMs is time since THIS activation, not raw millis(). Write a full
    // frame, including OFF pixels, and return whether any pixel changed. Pure
    // elapsed-time rendering ensures re-entry at zero restarts the effect.
    virtual bool render(PixelBuffer& pixels, std::uint32_t elapsedMs) const = 0;
};
