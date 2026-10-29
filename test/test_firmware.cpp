#include "Config.h"
#include "animations/AlternatingColors.h"
#include "animations/PoliceStrobe.h"
#include "animations/SynchronizedWaves.h"
#include "engine/AnimationScheduler.h"

#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            throw std::runtime_error(std::string(__FILE__) + ":" +             \
                                     std::to_string(__LINE__) + ": " #condition); \
        }                                                                       \
    } while (false)

namespace {

using Frame = std::array<CRGB, 16>;
// Independent literal expectations: no production mapping helper is reused.
const Frame ColorsInitial = {
    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
    CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
    CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red};
const Frame ColorsSwapped = {
    CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
    CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue};
const Frame StrobeBlue = {
    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue};
const Frame StrobeRed = {
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
    CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
    CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black};
const Frame Off{};

struct Fixture {
    Frame leds{};
    PixelBuffer pixels{leds.data(), leds.size()};
    // Fixed test inputs keep boundary expectations independent of the user's
    // configurable playback settings. Config.h still checks their validity.
    AlternatingColors colors{500};
    PoliceStrobe strobe{500, 50, 50};
    const Animation* const registry[2] = {&colors, &strobe};
    AnimationScheduler scheduler{registry, 2, 5000};
};

void expect(const Frame& actual, const Frame& expected) {
    for (std::size_t i = 0; i < actual.size(); ++i) {
        if (!(actual[i] == expected[i])) {
            throw std::runtime_error("Unexpected color at pixel " + std::to_string(i));
        }
    }
}

void colors_start() {
    Fixture f;
    f.pixels.fill(CRGB::Green);
    CHECK(f.colors.render(f.pixels, 0));
    expect(f.leds, ColorsInitial);
}

void colors_boundaries() {
    Fixture f;
    for (const auto t : {1U, 499U, 500U, 999U, 1000U, 1499U, 1500U, 4999U}) {
        f.colors.render(f.pixels, t);
        expect(f.leds, (t == 500 || t == 999 || t == 1500 || t == 4999)
                           ? ColorsSwapped : ColorsInitial);
    }
}

void colors_custom_timing() {
    Fixture f;
    AlternatingColors animation(73);
    animation.render(f.pixels, 72);
    expect(f.leds, ColorsInitial);
    animation.render(f.pixels, 73);
    expect(f.leds, ColorsSwapped);
    animation.render(f.pixels, 146);
    expect(f.leds, ColorsInitial);
}

void strobe_blue_on_off() {
    Fixture f;
    f.pixels.fill(CRGB::White);
    for (const auto t : {0U, 49U, 50U, 99U, 100U, 149U, 150U, 450U, 499U}) {
        f.strobe.render(f.pixels, t);
        expect(f.leds, (t == 0 || t == 49 || t == 100 || t == 149)
                           ? StrobeBlue : Off);
    }
}

void strobe_red_on_off() {
    Fixture f;
    for (const auto t : {500U, 549U, 550U, 599U, 600U, 949U, 950U, 999U, 1000U}) {
        f.strobe.render(f.pixels, t);
        if (t == 1000) {
            expect(f.leds, StrobeBlue);
        } else {
            expect(f.leds, (t == 500 || t == 549 || t == 600 || t == 949)
                               ? StrobeRed : Off);
        }
    }
}

void strobe_restarts_at_nondivisible_color_phase() {
    Fixture f;
    PoliceStrobe animation(275, 50, 50);
    animation.render(f.pixels, 274);
    expect(f.leds, Off);
    animation.render(f.pixels, 275);  // Global cycle would still be OFF.
    expect(f.leds, StrobeRed);
    animation.render(f.pixels, 324);
    expect(f.leds, StrobeRed);
    animation.render(f.pixels, 325);
    expect(f.leds, Off);
    animation.render(f.pixels, 550);
    expect(f.leds, StrobeBlue);
}

void strobe_asymmetric_duty_cycle() {
    Fixture f;
    PoliceStrobe animation(230, 75, 25);
    for (const auto t : {0U, 74U, 75U, 99U, 100U, 174U, 175U}) {
        animation.render(f.pixels, t);
        expect(f.leds, (t == 75 || t == 99 || t == 175) ? Off : StrobeBlue);
    }
    animation.render(f.pixels, 230);
    expect(f.leds, StrobeRed);
    animation.render(f.pixels, 304);
    expect(f.leds, StrobeRed);
    animation.render(f.pixels, 305);
    expect(f.leds, Off);
}

void scheduler_immediate_first_frame() {
    Fixture f;
    f.scheduler.begin(1234);
    CHECK(f.scheduler.update(1234, f.pixels));
    CHECK(f.scheduler.currentIndex() == 0);
    expect(f.leds, ColorsInitial);
    CHECK(!f.scheduler.update(1234, f.pixels));
}

void scheduler_rotation_boundaries() {
    Fixture f;
    f.scheduler.begin(1234);
    f.scheduler.update(1234, f.pixels);
    f.scheduler.update(6233, f.pixels);
    CHECK(f.scheduler.currentIndex() == 0);
    expect(f.leds, ColorsSwapped);
    CHECK(f.scheduler.update(6234, f.pixels));
    CHECK(f.scheduler.currentIndex() == 1);
    expect(f.leds, StrobeBlue);
    f.scheduler.update(6284, f.pixels);
    expect(f.leds, Off);
    f.scheduler.update(6334, f.pixels);
    expect(f.leds, StrobeBlue);
    f.scheduler.update(11233, f.pixels);
    CHECK(f.scheduler.currentIndex() == 1);
    CHECK(f.scheduler.update(11234, f.pixels));
    CHECK(f.scheduler.currentIndex() == 0);
    expect(f.leds, ColorsInitial);
}

void scheduler_reentry_resets_both_effects() {
    Fixture f;
    f.scheduler.update(0, f.pixels);
    f.scheduler.update(500, f.pixels);
    expect(f.leds, ColorsSwapped);
    f.scheduler.update(5000, f.pixels);
    f.scheduler.update(5550, f.pixels);
    expect(f.leds, Off);
    f.scheduler.update(10000, f.pixels);
    expect(f.leds, ColorsInitial);
    f.scheduler.update(15000, f.pixels);
    expect(f.leds, StrobeBlue);
}

void scheduler_no_stale_transition_pixels() {
    Fixture f;
    f.scheduler.update(0, f.pixels);
    f.pixels.fill(CRGB::Green);
    f.scheduler.update(5000, f.pixels);
    expect(f.leds, StrobeBlue);
    f.pixels.fill(CRGB::White);
    f.scheduler.update(10000, f.pixels);
    expect(f.leds, ColorsInitial);
}

void unchanged_frames_do_not_request_transmission() {
    Fixture f;
    CHECK(f.scheduler.update(0, f.pixels));
    for (std::uint32_t t = 1; t < 500; ++t) {
        CHECK(!f.scheduler.update(t, f.pixels));
    }
    CHECK(f.scheduler.update(500, f.pixels));
    CHECK(!f.scheduler.update(501, f.pixels));
    CHECK(f.scheduler.update(5000, f.pixels));
    CHECK(!f.scheduler.update(5049, f.pixels));
    CHECK(f.scheduler.update(5050, f.pixels));
    CHECK(!f.scheduler.update(5099, f.pixels));
    CHECK(f.scheduler.update(5100, f.pixels));
}

void skipped_loop_frames_use_elapsed_time() {
    Fixture f;
    f.scheduler.update(100, f.pixels);
    f.scheduler.update(1877, f.pixels);  // 1777 ms: swapped, no accumulated drift.
    expect(f.leds, ColorsSwapped);
    f.scheduler.update(2111, f.pixels);  // 2011 ms: original.
    expect(f.leds, ColorsInitial);
    f.scheduler.update(5100, f.pixels);
    f.scheduler.update(5777, f.pixels);  // 677 ms: red OFF.
    expect(f.leds, Off);
    f.scheduler.update(5803, f.pixels);  // 703 ms: red ON.
    expect(f.leds, StrobeRed);
}

void late_transition_starts_next_effect_at_zero() {
    Fixture f;
    f.scheduler.update(100, f.pixels);
    f.scheduler.update(12677, f.pixels);
    CHECK(f.scheduler.currentIndex() == 1);
    expect(f.leds, StrobeBlue);
    f.scheduler.update(12726, f.pixels);
    expect(f.leds, StrobeBlue);
    f.scheduler.update(12727, f.pixels);
    expect(f.leds, Off);
    f.scheduler.update(17676, f.pixels);
    CHECK(f.scheduler.currentIndex() == 1);
    f.scheduler.update(17677, f.pixels);
    CHECK(f.scheduler.currentIndex() == 0);
    expect(f.leds, ColorsInitial);
}

void scheduler_rollover_during_colors() {
    Fixture f;
    const std::uint32_t start = std::numeric_limits<std::uint32_t>::max() - 250;
    f.scheduler.begin(start);
    f.scheduler.update(start, f.pixels);
    f.scheduler.update(start + 499U, f.pixels);
    expect(f.leds, ColorsInitial);
    f.scheduler.update(start + 500U, f.pixels);
    expect(f.leds, ColorsSwapped);
    f.scheduler.update(start + 4999U, f.pixels);
    CHECK(f.scheduler.currentIndex() == 0);
    f.scheduler.update(start + 5000U, f.pixels);
    CHECK(f.scheduler.currentIndex() == 1);
    expect(f.leds, StrobeBlue);
    f.scheduler.update(start + 10000U, f.pixels);
    expect(f.leds, ColorsInitial);
}

void scheduler_rollover_during_strobe() {
    Fixture f;
    const std::uint32_t start = std::numeric_limits<std::uint32_t>::max() - 25;
    CHECK(f.scheduler.select(1, start));
    f.scheduler.update(start, f.pixels);
    f.scheduler.update(start + 49U, f.pixels);
    expect(f.leds, StrobeBlue);
    f.scheduler.update(start + 50U, f.pixels);
    expect(f.leds, Off);
    f.scheduler.update(start + 100U, f.pixels);
    expect(f.leds, StrobeBlue);
    f.scheduler.update(start + 500U, f.pixels);
    expect(f.leds, StrobeRed);
}

class IndividualPixels final : public Animation {
public:
    const char* name() const override { return "Individual pixels"; }
    bool render(PixelBuffer& pixels, std::uint32_t elapsedMs) const override {
        bool changed = false;
        for (std::size_t i = 0; i < pixels.size(); ++i) {
            const auto intensity = static_cast<std::uint8_t>(255 - (i % 4) * 50);
            const CRGB color(intensity, static_cast<std::uint8_t>(elapsedMs),
                             static_cast<std::uint8_t>(i * 11));
            changed = pixels.set(i, color) || changed;
        }
        return changed;
    }
};

void registry_supports_three_effects_and_individual_intensity() {
    Fixture f;
    IndividualPixels individual;
    const Animation* const registry[] = {&f.colors, &f.strobe, &individual};
    AnimationScheduler scheduler(registry, 3, 5000);
    scheduler.update(0, f.pixels);
    scheduler.update(5000, f.pixels);
    CHECK(scheduler.currentIndex() == 1);
    scheduler.update(10000, f.pixels);
    CHECK(scheduler.currentIndex() == 2);
    CHECK(f.leds[0] == CRGB(255, 0, 0));
    CHECK(f.leds[1] == CRGB(205, 0, 11));
    CHECK(f.leds[2] == CRGB(155, 0, 22));
    CHECK(f.leds[3] == CRGB(105, 0, 33));
    scheduler.update(10031, f.pixels);
    CHECK(f.leds[0] == CRGB(255, 31, 0));
    scheduler.update(15000, f.pixels);
    CHECK(scheduler.currentIndex() == 0);
    expect(f.leds, ColorsInitial);
}

void explicit_selection_and_next_restart() {
    Fixture f;
    f.scheduler.update(100, f.pixels);
    CHECK(f.scheduler.select(1, 200));
    f.scheduler.update(200, f.pixels);
    expect(f.leds, StrobeBlue);
    f.scheduler.update(750, f.pixels);
    expect(f.leds, Off);
    CHECK(f.scheduler.select(1, 751));
    f.scheduler.update(751, f.pixels);
    expect(f.leds, StrobeBlue);
    CHECK(!f.scheduler.select(2, 800));
    CHECK(f.scheduler.currentIndex() == 1);
    CHECK(f.scheduler.next(801));
    f.scheduler.update(801, f.pixels);
    expect(f.leds, ColorsInitial);
    f.scheduler.begin(802);  // Simulates resetting the scheduler at startup.
    f.scheduler.update(802, f.pixels);
    CHECK(f.scheduler.currentIndex() == 0);
    expect(f.leds, ColorsInitial);
}

void delayed_selection_gets_a_full_interval() {
    Fixture f;
    f.scheduler.update(0, f.pixels);
    CHECK(f.scheduler.select(1, 100));
    // A deferred update must not skip the requested animation or shorten it.
    f.scheduler.update(6200, f.pixels);
    CHECK(f.scheduler.currentIndex() == 1);
    expect(f.leds, StrobeBlue);
    f.scheduler.update(6249, f.pixels);
    expect(f.leds, StrobeBlue);
    f.scheduler.update(6250, f.pixels);
    expect(f.leds, Off);
    f.scheduler.update(11199, f.pixels);
    CHECK(f.scheduler.currentIndex() == 1);
    f.scheduler.update(11200, f.pixels);
    CHECK(f.scheduler.currentIndex() == 0);
    expect(f.leds, ColorsInitial);
}

void bounds_and_short_strings() {
    std::array<CRGB, 6> guarded{};
    guarded[0] = CRGB::Green;
    guarded[5] = CRGB::White;
    PixelBuffer pixels(guarded.data() + 1, 4);
    CHECK(!pixels.set(4, CRGB::Red));
    CHECK(!pixels.set(std::numeric_limits<std::size_t>::max(), CRGB::Red));
    AlternatingColors colors(500);
    colors.render(pixels, 0);
    for (std::size_t i = 1; i <= 4; ++i) {
        CHECK(guarded[i] == CRGB::Blue);
    }
    PoliceStrobe strobe(500, 50, 50);
    strobe.render(pixels, 500);
    for (std::size_t i = 1; i <= 4; ++i) {
        CHECK(guarded[i] == CRGB::Black);
    }
    CHECK(guarded[0] == CRGB::Green);
    CHECK(guarded[5] == CRGB::White);
    PixelBuffer empty(nullptr, 0);
    CHECK(!colors.render(empty, 0));
    CHECK(!strobe.render(empty, 0));
    CHECK(!empty.set(0, CRGB::Red));
}

void additional_pixels_default_to_off() {
    std::array<CRGB, 24> leds{};
    PixelBuffer pixels(leds.data(), leds.size());
    AlternatingColors colors(500);
    PoliceStrobe strobe(500, 50, 50);
    pixels.fill(CRGB::White);
    colors.render(pixels, 0);
    for (std::size_t i = 16; i < leds.size(); ++i) {
        CHECK(leds[i] == CRGB::Black);
    }
    pixels.fill(CRGB::White);
    strobe.render(pixels, 500);
    for (std::size_t i = 16; i < leds.size(); ++i) {
        CHECK(leds[i] == CRGB::Black);
    }
}

class PartialFrame final : public Animation {
public:
    const char* name() const override { return "Partial test frame"; }
    bool render(PixelBuffer& pixels, std::uint32_t) const override {
        return pixels.set(0, CRGB::Red);
    }
};

void transition_clears_even_a_partial_future_effect() {
    Fixture f;
    PartialFrame partial;
    const Animation* const registry[] = {&f.colors, &partial};
    AnimationScheduler scheduler(registry, 2, 5000);
    scheduler.update(0, f.pixels);
    CHECK(scheduler.update(5000, f.pixels));
    CHECK(f.leds[0] == CRGB::Red);
    for (std::size_t i = 1; i < f.leds.size(); ++i) {
        CHECK(f.leds[i] == CRGB::Black);
    }
}

void single_animation_restarts_every_interval() {
    Fixture f;
    const Animation* const registry[] = {&f.colors};
    AnimationScheduler scheduler(registry, 1, 750);
    scheduler.update(0, f.pixels);
    scheduler.update(500, f.pixels);
    expect(f.leds, ColorsSwapped);
    CHECK(scheduler.update(750, f.pixels));
    CHECK(scheduler.currentIndex() == 0);
    expect(f.leds, ColorsInitial);
    scheduler.update(1249, f.pixels);
    expect(f.leds, ColorsInitial);
}

void invalid_registries_fail_safely() {
    Fixture f;
    AnimationScheduler empty(nullptr, 0, 5000);
    CHECK(!empty.update(0, f.pixels));
    CHECK(!empty.next(0));
    CHECK(!empty.select(0, 0));
    AnimationScheduler zeroDuration(f.registry, 2, 0);
    CHECK(!zeroDuration.update(0, f.pixels));
    const Animation* const invalid[] = {&f.colors, nullptr};
    AnimationScheduler nullEntry(invalid, 2, 5000);
    CHECK(!nullEntry.update(0, f.pixels));
}

void invalid_effect_timings_fail_safely() {
    Fixture f;
    AlternatingColors colors(0);
    f.pixels.fill(CRGB::White);
    colors.render(f.pixels, 100);
    expect(f.leds, Off);
    const PoliceStrobe invalid[] = {
        {0, 50, 50}, {500, 0, 50}, {500, 50, 0},
        {500, std::numeric_limits<std::uint32_t>::max(), 1}};
    for (const auto& strobe : invalid) {
        f.pixels.fill(CRGB::White);
        strobe.render(f.pixels, 100);
        expect(f.leds, Off);
    }
}

std::uint8_t waveIntensity(const Frame& frame, std::size_t index) {
    return (index < 4 || index >= 12) ? frame[index].b : frame[index].r;
}

void expectWaveMinimum(const Frame& frame, std::uint8_t minimum) {
    for (std::size_t i = 0; i < frame.size(); ++i) {
        CHECK(frame[i] == ((i < 4 || i >= 12) ? CRGB(0, 0, minimum)
                                                          : CRGB(minimum, 0, 0)));
    }
}

void waves_start_at_minimum_without_stale_colors() {
    Fixture f;
    SynchronizedWaves waves(25, 255, 1000);
    f.pixels.fill(CRGB::Green);
    CHECK(waves.render(f.pixels, 0));
    expectWaveMinimum(f.leds, 25);
    CHECK(!waves.render(f.pixels, 0));
}

void waves_keep_colors_bounds_and_group_synchronization() {
    Fixture f;
    SynchronizedWaves waves(25, 255, 1000);
    for (std::uint32_t t = 0; t <= 2000; ++t) {
        waves.render(f.pixels, t);
        for (std::size_t i = 0; i < f.leds.size(); ++i) {
            CHECK(f.leds[i].g == 0);
            CHECK((i < 4 || i >= 12) ? f.leds[i].r == 0 : f.leds[i].b == 0);
            CHECK(waveIntensity(f.leds, i) >= 25);
            CHECK(waveIntensity(f.leds, i) <= 255);
        }
        // Independent expected directions: all four groups share position p.
        for (std::size_t p = 0; p < 4; ++p) {
            const auto intensity = f.leds[3 - p].b;
            CHECK(f.leds[4 + p].r == intensity);
            CHECK(f.leds[8 + p].r == intensity);
            CHECK(f.leds[15 - p].b == intensity);
        }
    }
}

void waves_originate_at_first_position() {
    Fixture f;
    SynchronizedWaves waves(25, 255, 1000);
    waves.render(f.pixels, 100);
    for (const auto i : {3U, 4U, 8U, 15U}) {
        CHECK(waveIntensity(f.leds, i) > 25);
    }
    for (const auto i : {0U, 1U, 2U, 5U, 6U, 7U, 9U, 10U, 11U, 12U, 13U, 14U}) {
        CHECK(waveIntensity(f.leds, i) == 25);
    }
}

void waves_peaks_travel_in_the_required_order() {
    Fixture f;
    SynchronizedWaves waves(25, 255, 1200);
    constexpr std::size_t ExpectedPixels[][4] = {
        {3, 4, 8, 15}, {2, 5, 9, 14}, {1, 6, 10, 13}, {0, 7, 11, 12}};
    constexpr std::uint32_t PeakTimes[] = {300, 500, 700, 900};
    for (std::size_t position = 0; position < 4; ++position) {
        waves.render(f.pixels, PeakTimes[position]);
        for (const auto index : ExpectedPixels[position]) {
            CHECK(waveIntensity(f.leds, index) == 255);
        }
        for (std::size_t other = 0; other < 4; ++other) {
            if (other != position) {
                CHECK(waveIntensity(f.leds, ExpectedPixels[other][0]) < 100);
            }
        }
    }
}

void waves_brighten_and_dim_smoothly() {
    Fixture f;
    SynchronizedWaves waves(25, 255, 1200);
    // A 1200 ms cycle puts all four starts, peaks and ends on integer ms.
    for (std::size_t position = 0; position < 4; ++position) {
        const auto start = static_cast<std::uint32_t>(position * 200);
        std::uint8_t previous = 25;
        for (std::uint32_t offset = 0; offset <= 600; ++offset) {
            waves.render(f.pixels, start + offset);
            const auto current = f.leds[3 - position].b;
            const int delta = static_cast<int>(current) - previous;
            CHECK(delta >= -3 && delta <= 3);  // No jumps at pulse edges.
            CHECK(offset <= 300 ? current >= previous : current <= previous);
            if (offset == 0 || offset == 600) {
                CHECK(current == 25);
            } else if (offset == 300) {
                CHECK(current == 255);
            }
            previous = current;
        }
    }
}

void waves_adjacent_pulses_overlap_but_ends_do_not() {
    Fixture f;
    SynchronizedWaves waves(25, 255, 1200);
    for (std::size_t position = 0; position < 3; ++position) {
        waves.render(f.pixels, static_cast<std::uint32_t>(400 + position * 200));
        CHECK(f.leds[3 - position].b > 190);
        CHECK(f.leds[2 - position].b > 190);
    }
    for (std::uint32_t t = 0; t < 1200; ++t) {
        waves.render(f.pixels, t);
        CHECK(f.leds[3].b == 25 || f.leds[0].b == 25);
    }
}

void waves_repeat_with_a_smooth_cycle_seam() {
    Fixture f;
    SynchronizedWaves waves(25, 255, 1000);
    waves.render(f.pixels, 999);
    const Frame end = f.leds;
    waves.render(f.pixels, 1000);
    expectWaveMinimum(f.leds, 25);
    expect(f.leds, end);  // Both sides quantize to minimum with zero edge slope.
    waves.render(f.pixels, 1001);
    expectWaveMinimum(f.leds, 25);
    for (std::uint32_t t = 0; t < 1000; t += 7) {
        waves.render(f.pixels, t);
        const Frame initial = f.leds;
        waves.render(f.pixels, t + 1000);
        expect(f.leds, initial);
        waves.render(f.pixels, t + 3000);
        expect(f.leds, initial);
    }
}

void waves_zero_minimum_starts_dark() {
    Fixture f;
    SynchronizedWaves waves(0, 255, 1000);
    f.pixels.fill(CRGB::White);
    waves.render(f.pixels, 0);
    expect(f.leds, Off);
    waves.render(f.pixels, 100);
    CHECK(f.leds[3].b > 0);
    CHECK(f.leds[2].b == 0);
    waves.render(f.pixels, 1000);
    expect(f.leds, Off);
}

void waves_configurable_ranges_and_degenerate_inputs() {
    Fixture f;
    SynchronizedWaves narrow(80, 120, 1000);
    for (std::uint32_t t = 0; t <= 1000; ++t) {
        narrow.render(f.pixels, t);
        for (std::size_t i = 0; i < f.leds.size(); ++i) {
            CHECK(waveIntensity(f.leds, i) >= 80);
            CHECK(waveIntensity(f.leds, i) <= 120);
        }
    }
    narrow.render(f.pixels, 250);
    CHECK(f.leds[3].b == 120);
    const SynchronizedWaves flat[] = {
        {70, 70, 1000}, {180, 10, 1000}, {0, 0, 1000},
        {255, 255, 1000}, {25, 255, 0}};
    constexpr std::uint8_t ExpectedMinima[] = {70, 180, 0, 255, 25};
    for (std::size_t i = 0; i < sizeof(flat) / sizeof(flat[0]); ++i) {
        for (const auto t : {0U, 250U, 750U, 1000U, 4000U}) {
            flat[i].render(f.pixels, t);
            expectWaveMinimum(f.leds, ExpectedMinima[i]);
        }
    }
}

void waves_custom_short_odd_and_long_cycles() {
    Fixture f;
    for (const auto cycle : {1U, 7U, 733U, 1200U, 3000U}) {
        SynchronizedWaves waves(25, 255, cycle);
        for (std::uint32_t t = 0; t < cycle; ++t) {
            waves.render(f.pixels, t);
            const Frame initial = f.leds;
            waves.render(f.pixels, t + cycle);
            expect(f.leds, initial);
        }
    }
    const auto maximum = std::numeric_limits<std::uint32_t>::max();
    SynchronizedWaves longCycle(25, 255, maximum);
    longCycle.render(f.pixels, maximum / 4);
    CHECK(f.leds[3].b == 255);
    longCycle.render(f.pixels, maximum - 1);
    expectWaveMinimum(f.leds, 25);
    longCycle.render(f.pixels, maximum);
    expectWaveMinimum(f.leds, 25);
}

void waves_leave_output_unscaled_for_global_brightness() {
    Fixture f;
    SynchronizedWaves waves(25, 255, 1000);
    waves.render(f.pixels, 250);
    // The renderer must provide the full raw channel to FastLED, which applies
    // the unchanged GlobalBrightness cap once at transmission time.
    CHECK(f.leds[3] == CRGB(0, 0, 255));
    CHECK(f.leds[4] == CRGB(255, 0, 0));
    CHECK(config::GlobalBrightness == 50);
}

void waves_write_only_within_the_buffer() {
    std::array<CRGB, 6> guarded{};
    guarded[0] = CRGB::Green;
    guarded[5] = CRGB::White;
    PixelBuffer shortString(guarded.data() + 1, 4);
    SynchronizedWaves waves(25, 255, 1000);
    waves.render(shortString, 250);
    CHECK(guarded[0] == CRGB::Green);
    CHECK(guarded[5] == CRGB::White);
    CHECK(guarded[4] == CRGB(0, 0, 255));
    std::array<CRGB, 24> extended{};
    PixelBuffer longString(extended.data(), extended.size());
    longString.fill(CRGB::White);
    waves.render(longString, 250);
    for (std::size_t i = 16; i < extended.size(); ++i) {
        CHECK(extended[i] == CRGB::Black);
    }
    PixelBuffer empty(nullptr, 0);
    CHECK(!waves.render(empty, 250));
}

void configured_three_effect_rotation_and_wave_reentry() {
    Fixture f;
    PoliceStrobe strobe(config::PoliceColorPhaseMs, config::PoliceStrobeOnMs,
                        config::PoliceStrobeOffMs);
    SynchronizedWaves waves(config::WaveMinBrightness, config::WaveMaxBrightness,
                            config::WaveCycleMs);
    const Animation* const registry[] = {&f.colors, &strobe, &waves};
    AnimationScheduler scheduler(registry, 3, config::AnimationRotationMs);
    scheduler.update(0, f.pixels);
    expect(f.leds, ColorsInitial);
    scheduler.update(500, f.pixels);
    expect(f.leds, ColorsSwapped);
    scheduler.update(4000, f.pixels);
    expect(f.leds, StrobeBlue);
    scheduler.update(4025, f.pixels);
    expect(f.leds, Off);
    scheduler.update(4050, f.pixels);
    expect(f.leds, StrobeBlue);
    scheduler.update(4500, f.pixels);
    expect(f.leds, StrobeRed);
    CHECK(scheduler.update(8000, f.pixels));
    CHECK(scheduler.currentIndex() == 2);
    expectWaveMinimum(f.leds, 25);
    scheduler.update(8250, f.pixels);
    CHECK(f.leds[3].b == 255);
    scheduler.update(11999, f.pixels);
    CHECK(scheduler.currentIndex() == 2);
    CHECK(scheduler.update(12000, f.pixels));
    CHECK(scheduler.currentIndex() == 0);
    expect(f.leds, ColorsInitial);
    scheduler.update(16000, f.pixels);
    scheduler.update(20000, f.pixels);
    CHECK(scheduler.currentIndex() == 2);
    expectWaveMinimum(f.leds, 25);
}

void waves_scheduler_rollover_and_skipped_frames() {
    Fixture f;
    SynchronizedWaves waves(25, 255, 1000);
    const Animation* const registry[] = {&f.colors, &f.strobe, &waves};
    AnimationScheduler scheduler(registry, 3, 4000);
    const auto start = std::numeric_limits<std::uint32_t>::max() - 300U;
    CHECK(scheduler.select(2, start));
    scheduler.update(start, f.pixels);
    expectWaveMinimum(f.leds, 25);
    scheduler.update(start + 250U, f.pixels);
    CHECK(f.leds[3].b == 255);
    scheduler.update(start + 750U, f.pixels);  // Deliberately skip frames and wrap.
    CHECK(f.leds[0].b == 255);
    scheduler.update(start + 1000U, f.pixels);
    expectWaveMinimum(f.leds, 25);
    scheduler.update(start + 3999U, f.pixels);
    CHECK(scheduler.currentIndex() == 2);
    scheduler.update(start + 4000U, f.pixels);
    CHECK(scheduler.currentIndex() == 0);
    expect(f.leds, ColorsInitial);
}

}  // namespace

int main() {
    struct Test { const char* name; void (*run)(); };
    const Test tests[] = {
        {"colors start mapping", colors_start},
        {"colors phase boundaries", colors_boundaries},
        {"colors configurable phase", colors_custom_timing},
        {"blue strobe boundaries", strobe_blue_on_off},
        {"red strobe boundaries", strobe_red_on_off},
        {"strobe phase clock reset", strobe_restarts_at_nondivisible_color_phase},
        {"asymmetric strobe duty cycle", strobe_asymmetric_duty_cycle},
        {"immediate first frame", scheduler_immediate_first_frame},
        {"5000 ms rotation and wrap to first", scheduler_rotation_boundaries},
        {"effect re-entry resets", scheduler_reentry_resets_both_effects},
        {"no stale transition pixels", scheduler_no_stale_transition_pixels},
        {"unchanged frames skip transmission", unchanged_frames_do_not_request_transmission},
        {"slow loops preserve phase", skipped_loop_frames_use_elapsed_time},
        {"late transition begins at zero", late_transition_starts_next_effect_at_zero},
        {"unsigned rollover in colors and rotation", scheduler_rollover_during_colors},
        {"unsigned rollover during strobe", scheduler_rollover_during_strobe},
        {"three effects and independent pixels", registry_supports_three_effects_and_individual_intensity},
        {"explicit selection and next", explicit_selection_and_next_restart},
        {"deferred selection gets a full interval", delayed_selection_gets_a_full_interval},
        {"bounds and shortened strings", bounds_and_short_strings},
        {"additional pixels are off", additional_pixels_default_to_off},
        {"defensive activation clear", transition_clears_even_a_partial_future_effect},
        {"single effect rotation", single_animation_restarts_every_interval},
        {"invalid registries", invalid_registries_fail_safely},
        {"invalid effect timings", invalid_effect_timings_fail_safely},
        {"waves initial minimum and stale colors", waves_start_at_minimum_without_stale_colors},
        {"waves fixed colors bounds and synchronization", waves_keep_colors_bounds_and_group_synchronization},
        {"waves start at the first position", waves_originate_at_first_position},
        {"waves peak direction and localization", waves_peaks_travel_in_the_required_order},
        {"waves smooth rise and fall", waves_brighten_and_dim_smoothly},
        {"waves adjacent overlap and separated ends", waves_adjacent_pulses_overlap_but_ends_do_not},
        {"waves periodicity and smooth seam", waves_repeat_with_a_smooth_cycle_seam},
        {"waves zero minimum", waves_zero_minimum_starts_dark},
        {"waves custom and degenerate brightness ranges", waves_configurable_ranges_and_degenerate_inputs},
        {"waves custom and full-width cycle durations", waves_custom_short_odd_and_long_cycles},
        {"waves leave global brightness to FastLED", waves_leave_output_unscaled_for_global_brightness},
        {"waves buffer bounds and extra pixels", waves_write_only_within_the_buffer},
        {"configured rotation 1 2 3 and restart", configured_three_effect_rotation_and_wave_reentry},
        {"waves rollover and skipped frames", waves_scheduler_rollover_and_skipped_frames},
    };
    unsigned passed = 0;
    for (const auto& test : tests) {
        try {
            test.run();
            ++passed;
            std::cout << "PASS: " << test.name << '\n';
        } catch (const std::exception& error) {
            std::cerr << "FAIL: " << test.name << ": " << error.what() << '\n';
            return 1;
        }
    }
    std::cout << passed << " tests passed\n";
    return 0;
}
