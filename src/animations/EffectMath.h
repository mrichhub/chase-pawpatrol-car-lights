#pragma once

#include <FastLED.h>
#include <cstddef>
#include <cstdint>

// Small, stateless helpers shared by the new effects. No scheduler or physical
// group ownership lives here; renderers still assign each pixel independently.
namespace effect_math {
constexpr float Pi = 3.14159265358979323846f;
constexpr float TwoPi = 2.0f * Pi;

inline float phase(std::uint32_t elapsedMs, std::uint32_t cycleMs) {
    return cycleMs == 0 ? 0.0f : static_cast<float>(elapsedMs % cycleMs) / cycleMs;
}

inline float smoothstep(float value) {
    if (value <= 0.0f) {
        return 0.0f;
    }
    if (value >= 1.0f) {
        return 1.0f;
    }
    return value * value * (3.0f - 2.0f * value);
}

inline std::uint8_t intensity(std::uint8_t minimum, std::uint8_t maximum,
                              float amount) {
    if (maximum <= minimum || amount <= 0.0f) {
        return minimum;
    }
    if (amount >= 1.0f) {
        return maximum;
    }
    return static_cast<std::uint8_t>(minimum +
        static_cast<unsigned>((maximum - minimum) * amount + 0.5f));
}

// Default fixed colors for effects 4, 5, 7, 8, 9. Additional pixels stay OFF.
inline CRGB fixedColor(std::size_t index, std::uint8_t brightness) {
    if (index >= 16) {
        return CRGB::Black;
    }
    return (index < 4 || index >= 12) ? CRGB(0, 0, brightness)
                                     : CRGB(brightness, 0, 0);
}
}  // namespace effect_math
