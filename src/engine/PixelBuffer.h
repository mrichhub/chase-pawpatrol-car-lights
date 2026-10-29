#pragma once

#include <FastLED.h>
#include <cstddef>

// A non-owning view of the one CRGB array used by FastLED. There are no groups
// here: every pixel can have its own RGB values (and therefore intensity).
class PixelBuffer {
public:
    PixelBuffer(CRGB* data, std::size_t count) : data_(data), count_(count) {}

    std::size_t size() const { return count_; }

    // Return true only if the buffer actually changed. Invalid indices are
    // ignored, so a shortened string cannot be written past its end.
    bool set(std::size_t index, const CRGB& color) {
        if (index >= count_ || data_ == nullptr || data_[index] == color) {
            return false;
        }
        data_[index] = color;
        return true;
    }

    bool fill(const CRGB& color) {
        bool changed = false;
        for (std::size_t i = 0; i < count_; ++i) {
            changed = set(i, color) || changed;
        }
        return changed;
    }

private:
    CRGB* data_;
    std::size_t count_;
};
