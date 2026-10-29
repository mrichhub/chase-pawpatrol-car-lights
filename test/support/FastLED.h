#pragma once

// Host-only color shim. The production sources are compiled unchanged; only
// FastLED's hardware-dependent header is replaced. The ESP32 build uses the
// real library and verifies the actual FastLED API integration separately.
#include <cstdint>

enum EOrder { RGB, RBG, GRB, GBR, BRG, BGR };

struct CRGB {
    enum HTMLColorCode : std::uint32_t {
        Black = 0x000000,
        Blue = 0x0000FF,
        Red = 0xFF0000,
        Green = 0x008000,
        White = 0xFFFFFF
    };

    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;

    CRGB() = default;
    CRGB(std::uint8_t red, std::uint8_t green, std::uint8_t blue)
        : r(red), g(green), b(blue) {}
    CRGB(HTMLColorCode color)
        : r(static_cast<std::uint8_t>(color >> 16)),
          g(static_cast<std::uint8_t>(color >> 8)),
          b(static_cast<std::uint8_t>(color)) {}

    bool operator==(const CRGB& other) const {
        return r == other.r && g == other.g && b == other.b;
    }
};
