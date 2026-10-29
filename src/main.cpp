#include <Arduino.h>
#include <FastLED.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Config.h"
#include "animations/AlternatingColors.h"
#include "animations/PoliceStrobe.h"
#include "animations/SynchronizedWaves.h"
#include "engine/AnimationScheduler.h"

namespace {
CRGB leds[config::LedCount];
PixelBuffer pixels(leds, config::LedCount);

const AlternatingColors alternatingColors(config::AlternatingColorsPhaseMs);
const PoliceStrobe policeStrobe(config::PoliceColorPhaseMs,
                               config::PoliceStrobeOnMs,
                               config::PoliceStrobeOffMs);
const SynchronizedWaves synchronizedWaves(config::WaveMinBrightness,
                                          config::WaveMaxBrightness,
                                          config::WaveCycleMs);

// Add future animations here, in playback order. The scheduler is unchanged.
const Animation* const animations[] = {&alternatingColors, &policeStrobe,
                                       &synchronizedWaves};
AnimationScheduler scheduler(animations, sizeof(animations) / sizeof(animations[0]),
                             config::AnimationRotationMs);

void logSelection() {
    if constexpr (config::SerialDebug) {
        Serial.printf("Animation %u: %s\n",
                      static_cast<unsigned>(scheduler.currentIndex() + 1),
                      scheduler.currentName());
    }
}
}  // namespace

void setup() {
    if constexpr (config::SerialDebug) {
        Serial.begin(config::SerialBaud);  // No wait for a serial connection.
    }

    FastLED.addLeds<CHASE_LED_CHIPSET, config::LedDataPin, config::LedColorOrder>(
        leds, static_cast<int>(config::LedCount));
    FastLED.setBrightness(config::GlobalBrightness);
    // Dithering requires continuous retransmission. Disable it so an unchanged
    // frame stays stable while we deliberately avoid redundant show() calls.
    FastLED.setDither(DISABLE_DITHER);

    pixels.fill(CRGB::Black);
    const auto nowMs = static_cast<std::uint32_t>(millis());
    scheduler.begin(nowMs);
    if (scheduler.update(nowMs, pixels)) {
        FastLED.show();
    }
    logSelection();
}

void loop() {
    const auto previousIndex = scheduler.currentIndex();
    if (scheduler.update(static_cast<std::uint32_t>(millis()), pixels)) {
        FastLED.show();
    }
    if (scheduler.currentIndex() != previousIndex) {
        logSelection();
    }

    // Idle for one RTOS tick (1 ms on this target) to let other tasks run and
    // avoid busy-spinning. This is only a polling yield: every animation and
    // transition is timed by millis(), never by counting task sleeps.
    vTaskDelay(1);
}
