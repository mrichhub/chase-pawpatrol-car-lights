#pragma once

#include <FastLED.h>
#include <cstddef>
#include <cstdint>
#include <limits>

// FastLED takes the chipset as a template argument, so keep it as a macro.
#define CHASE_LED_CHIPSET WS2811

namespace config {

constexpr std::uint8_t LedDataPin = 18;
constexpr std::size_t LedCount = 16;
constexpr EOrder LedColorOrder = RGB;  // Change to GRB, BRG, etc. if needed.
constexpr std::uint8_t GlobalBrightness = 50;  // 0 = off, 255 = maximum.

constexpr std::uint32_t AnimationRotationMs = 4000;
constexpr std::uint32_t AlternatingColorsPhaseMs = 500;
constexpr std::uint32_t PoliceColorPhaseMs = 500;
constexpr std::uint32_t PoliceStrobeOnMs = 25;
constexpr std::uint32_t PoliceStrobeOffMs = 25;

constexpr bool SerialDebug = false;
constexpr unsigned long SerialBaud = 115200;

static_assert(LedCount > 0, "At least one LED is required");
static_assert(LedCount <= static_cast<std::size_t>(std::numeric_limits<int>::max()),
              "LED count must fit FastLED's integer count");
static_assert(AnimationRotationMs > 0, "Rotation duration must be positive");
static_assert(AlternatingColorsPhaseMs > 0, "Color phase must be positive");
static_assert(PoliceColorPhaseMs > 0, "Police color phase must be positive");
static_assert(PoliceStrobeOnMs > 0 && PoliceStrobeOffMs > 0,
              "Both strobe durations must be positive");
static_assert(PoliceStrobeOnMs <=
                  std::numeric_limits<std::uint32_t>::max() - PoliceStrobeOffMs,
              "Strobe cycle must fit in uint32_t");

}  // namespace config
