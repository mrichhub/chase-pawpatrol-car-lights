#pragma once

#include "animations/Animation.h"
#include <cstddef>
#include <cstdint>

class AnimationScheduler {
public:
    // Registry and animation instances must outlive this scheduler. A static
    // array in main.cpp provides that lifetime without heap allocation.
    AnimationScheduler(const Animation* const* animations, std::size_t count,
                       std::uint32_t rotationMs);

    void begin(std::uint32_t nowMs);

    // Returns true when the caller must transmit the buffer with FastLED.show().
    bool update(std::uint32_t nowMs, PixelBuffer& pixels);

    // Selection is separate from rendering so future controls can call these
    // functions, then let update() render at zero on the next loop iteration.
    // Its full rotation interval starts with that first rendered frame.
    bool select(std::size_t index, std::uint32_t nowMs);
    bool next(std::uint32_t nowMs);

    std::size_t currentIndex() const { return currentIndex_; }
    const char* currentName() const;

private:
    const Animation* const* animations_;
    const std::size_t count_;
    const std::uint32_t rotationMs_;
    bool valid_ = false;
    bool started_ = false;
    bool firstFrame_ = false;
    std::size_t currentIndex_ = 0;
    std::uint32_t activatedAtMs_ = 0;
};
