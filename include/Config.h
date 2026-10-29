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

constexpr std::uint8_t WaveMinBrightness = 10;
constexpr std::uint8_t WaveMaxBrightness = 255;
constexpr std::uint32_t WaveCycleMs = 1000;

// Animation 4: three flashes per color, then a longer pause.
constexpr std::uint32_t TripleFlashOnMs = 75;
constexpr std::uint32_t TripleFlashOffMs = 75;
constexpr std::uint32_t TripleFlashPauseMs = 150;
constexpr std::uint8_t TripleFlashBrightness = 255;

// Animation 5: full forward/back scanner cycle and trailing glow strength.
constexpr std::uint32_t ScannerRoundTripMs = 600;
constexpr std::uint8_t ScannerMinBrightness = 0;
constexpr std::uint8_t ScannerMaxBrightness = 255;
constexpr std::uint8_t ScannerTrailStrength = 110;

// Animation 6: immediate checkerboard color swaps.
constexpr std::uint32_t CheckerboardPhaseMs = 200;
constexpr std::uint8_t CheckerboardBrightness = 255;

// Animation 7: center rise, expansion, contraction, and fade.
constexpr std::uint32_t CenterOutCycleMs = 700;
constexpr std::uint8_t CenterOutMinBrightness = 0;
constexpr std::uint8_t CenterOutMaxBrightness = 255;

// Animation 8: one complete circular rotation.
constexpr std::uint32_t BeaconRotationMs = 800;
constexpr std::uint8_t BeaconMinBrightness = 0;
constexpr std::uint8_t BeaconMaxBrightness = 255;

// Animation 9: entry, travel, tail exit, and rest in one cycle.
constexpr std::uint32_t CometCycleMs = 1000;
constexpr std::uint8_t CometMinBrightness = 0;
constexpr std::uint8_t CometMaxBrightness = 255;
constexpr std::uint8_t CometTrailLength = 3;

constexpr bool SerialDebug = true;
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
static_assert(WaveCycleMs > 0, "Wave cycle must be positive");
static_assert(TripleFlashOnMs > 0 && TripleFlashOffMs > 0 && TripleFlashPauseMs > 0,
              "Triple flash intervals must be positive");
static_assert(2ULL * (3ULL * TripleFlashOnMs + 2ULL * TripleFlashOffMs +
                     TripleFlashPauseMs) <= std::numeric_limits<std::uint32_t>::max(),
              "Triple flash sequence must fit in uint32_t");
static_assert(ScannerRoundTripMs > 0, "Scanner cycle must be positive");
static_assert(CheckerboardPhaseMs > 0, "Checkerboard phase must be positive");
static_assert(CenterOutCycleMs > 0, "Center-out cycle must be positive");
static_assert(BeaconRotationMs > 0, "Beacon cycle must be positive");
static_assert(CometCycleMs > 0, "Comet cycle must be positive");

}  // namespace config
