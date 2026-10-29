// Compile the real registry, setup(), and loop() against host-only hardware
// stubs. Tests below inspect the real production instances, not a second list.
#include "main.cpp"

#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#define CHECK_NEW(condition)                                                     \
    do {                                                                        \
        if (!(condition)) {                                                     \
            throw std::runtime_error(std::string(__FILE__) + ":" +              \
                                     std::to_string(__LINE__) + ": " #condition); \
        }                                                                       \
    } while (false)

namespace {
struct NewFrame {
    std::array<CRGB, 16> data{};
    PixelBuffer pixels{data.data(), data.size()};
    std::uint8_t intensity(std::size_t index) const {
        return (index < 4 || index >= 12) ? data[index].b : data[index].r;
    }
};

void expectFixed(const NewFrame& frame, std::uint8_t value) {
    for (std::size_t i = 0; i < frame.data.size(); ++i) {
        CHECK_NEW(frame.data[i] == ((i < 4 || i >= 12) ? CRGB(0, 0, value)
                                                       : CRGB(value, 0, 0)));
    }
}

void expectFlash(const NewFrame& frame, int activeColor, std::uint8_t value = 255) {
    for (std::size_t i = 0; i < frame.data.size(); ++i) {
        const bool blue = i < 4 || i >= 12;
        const CRGB expected = activeColor == (blue ? 1 : 2)
            ? (blue ? CRGB(0, 0, value) : CRGB(value, 0, 0)) : CRGB::Black;
        CHECK_NEW(frame.data[i] == expected);
    }
}

void triple_flash_boundaries_and_exact_flash_count() {
    NewFrame frame;
    PoliceTripleFlash effect(75, 75, 150, 255);
    unsigned blueStarts = 0;
    unsigned redStarts = 0;
    bool previousBlue = false;
    bool previousRed = false;
    for (std::uint32_t t = 0; t < 1050; ++t) {
        effect.render(frame.pixels, t);
        const bool blue = frame.data[0].b != 0;
        const bool red = frame.data[4].r != 0;
        if (blue && !previousBlue) { ++blueStarts; }
        if (red && !previousRed) { ++redStarts; }
        const auto local = t % 525;
        const bool expectedOn = local < 75 || (local >= 150 && local < 225) ||
                                (local >= 300 && local < 375);
        expectFlash(frame, expectedOn ? (t < 525 ? 1 : 2) : 0);
        previousBlue = blue;
        previousRed = red;
    }
    CHECK_NEW(blueStarts == 3 && redStarts == 3);
    effect.render(frame.pixels, 1050);
    expectFlash(frame, 1);
    effect.render(frame.pixels, 824);  // Skipped straight into red OFF.
    expectFlash(frame, 0);
    effect.render(frame.pixels, 825);
    expectFlash(frame, 2);
}

void triple_flash_custom_timing_and_brightness() {
    NewFrame frame;
    PoliceTripleFlash effect(30, 10, 100, 90);  // 210 ms per color.
    for (const auto t : {0U, 29U, 40U, 69U, 80U, 109U}) {
        effect.render(frame.pixels, t);
        expectFlash(frame, 1, 90);
    }
    for (const auto t : {30U, 39U, 70U, 79U, 110U, 209U}) {
        effect.render(frame.pixels, t);
        expectFlash(frame, 0);
    }
    effect.render(frame.pixels, 210);
    expectFlash(frame, 2, 90);
    effect.render(frame.pixels, 420);
    expectFlash(frame, 1, 90);
    PoliceTripleFlash wide(std::numeric_limits<std::uint32_t>::max(), 75, 150, 80);
    wide.render(frame.pixels, 1234);  // Intermediate sums must not overflow.
    expectFlash(frame, 1, 80);
}

void scanner_reverses_and_interpolates() {
    NewFrame frame;
    ScannerSweep effect(600, 0, 255, 110);
    effect.render(frame.pixels, 0);
    CHECK_NEW(frame.data[0].b == 255);
    effect.render(frame.pixels, 100);
    CHECK_NEW(frame.data[1].b > frame.data[0].b);
    CHECK_NEW(frame.data[1].b > frame.data[2].b);
    effect.render(frame.pixels, 150);
    CHECK_NEW(frame.data[1].b >= 127 && frame.data[2].b >= 127);
    effect.render(frame.pixels, 200);
    CHECK_NEW(frame.data[2].b > frame.data[1].b);
    effect.render(frame.pixels, 300);
    CHECK_NEW(frame.data[3].b == 255);
    effect.render(frame.pixels, 400);
    CHECK_NEW(frame.data[2].b > frame.data[3].b);
    effect.render(frame.pixels, 500);
    CHECK_NEW(frame.data[1].b > frame.data[2].b);
    effect.render(frame.pixels, 600);
    CHECK_NEW(frame.data[0].b == 255);
}

void scanner_trail_control_is_visible_and_bounded() {
    NewFrame frame;
    ScannerSweep clean(600, 0, 255, 0);
    ScannerSweep trailing(600, 0, 255, 255);
    clean.render(frame.pixels, 200);
    const auto withoutTail = frame.data;
    trailing.render(frame.pixels, 200);
    CHECK_NEW(frame.data[1].b > withoutTail[1].b);
    CHECK_NEW(frame.data[2].b >= frame.data[1].b);
    CHECK_NEW(frame.data[2].b >= frame.data[3].b);
}

void checkerboard_exact_mapping_and_restart() {
    NewFrame frame;
    RedBlueCheckerboard effect(200, 255);
    const std::array<CRGB, 16> initial = {
        CRGB::Red, CRGB::Blue, CRGB::Red, CRGB::Blue,
        CRGB::Blue, CRGB::Red, CRGB::Blue, CRGB::Red,
        CRGB::Red, CRGB::Blue, CRGB::Red, CRGB::Blue,
        CRGB::Blue, CRGB::Red, CRGB::Blue, CRGB::Red};
    for (const auto t : {0U, 199U, 200U, 399U, 400U, 0U}) {
        effect.render(frame.pixels, t);
        for (std::size_t i = 0; i < frame.data.size(); ++i) {
            const auto expected = t == 200 || t == 399
                ? (initial[i] == CRGB::Red ? CRGB::Blue : CRGB::Red) : initial[i];
            CHECK_NEW(frame.data[i] == expected);
        }
    }
    RedBlueCheckerboard custom(73, 80);
    custom.render(frame.pixels, 0);
    CHECK_NEW(frame.data[0] == CRGB(80, 0, 0));
    custom.render(frame.pixels, 73);
    CHECK_NEW(frame.data[0] == CRGB(0, 0, 80));
}

void center_out_origin_expansion_contraction_and_fade() {
    NewFrame frame;
    CenterOutExplosion effect(700, 0, 255);
    effect.render(frame.pixels, 0);
    expectFixed(frame, 0);
    effect.render(frame.pixels, 87);
    CHECK_NEW(frame.data[1].b > 0 && frame.data[2].b > 0);
    CHECK_NEW(frame.data[0].b == 0 && frame.data[3].b == 0);
    effect.render(frame.pixels, 175);
    CHECK_NEW(frame.data[1].b == 255 && frame.data[0].b == 0);
    effect.render(frame.pixels, 262);
    CHECK_NEW(frame.data[0].b > 100 && frame.data[1].b > 100);
    effect.render(frame.pixels, 350);
    CHECK_NEW(frame.data[0].b == 255 && frame.data[1].b == 0);
    effect.render(frame.pixels, 437);
    CHECK_NEW(frame.data[0].b > 100 && frame.data[1].b > 100);
    effect.render(frame.pixels, 525);
    CHECK_NEW(frame.data[1].b == 255 && frame.data[0].b == 0);
    effect.render(frame.pixels, 612);
    CHECK_NEW(frame.data[1].b > 0 && frame.data[1].b < 255);
    CHECK_NEW(frame.data[0].b == 0);
    effect.render(frame.pixels, 700);
    expectFixed(frame, 0);
}

void beacon_rotates_forward_and_wraps_circularly() {
    NewFrame frame;
    RotatingBeacon effect(800, 0, 255);
    for (std::size_t p = 0; p < 4; ++p) {
        effect.render(frame.pixels, static_cast<std::uint32_t>(p * 200));
        CHECK_NEW(frame.data[p].b == 255);
        CHECK_NEW(frame.data[(p + 1) % 4].b > 0);
        CHECK_NEW(frame.data[(p + 3) % 4].b > 0);
        CHECK_NEW(frame.data[(p + 2) % 4].b == 0);
    }
    effect.render(frame.pixels, 700);
    CHECK_NEW(frame.data[3].b > 180 && frame.data[0].b > 180);
    CHECK_NEW(frame.data[1].b == 0 && frame.data[2].b == 0);
    effect.render(frame.pixels, 800);
    CHECK_NEW(frame.data[0].b == 255);
}

void comet_has_an_asymmetric_tail_and_leaves_before_restart() {
    NewFrame frame;
    CometChase effect(1000, 0, 255, 3);
    effect.render(frame.pixels, 0);
    expectFixed(frame, 0);
    constexpr std::size_t HeadPixels[] = {3, 2, 1, 0};
    for (std::size_t p = 0; p < 4; ++p) {
        effect.render(frame.pixels, static_cast<std::uint32_t>((p + 1) * 125));
        CHECK_NEW(frame.data[HeadPixels[p]].b == 255);
    }
    effect.render(frame.pixels, 375);  // Head position 2; two tail pixels behind.
    CHECK_NEW(frame.data[1].b == 255);
    CHECK_NEW(frame.data[2].b > frame.data[3].b);
    CHECK_NEW(frame.data[3].b > 0);
    CHECK_NEW(frame.data[0].b == 0);  // Minimal illumination in front.
    effect.render(frame.pixels, 625);  // Head has left the physical group.
    CHECK_NEW(frame.data[0].b > 0 && frame.data[0].b < 255);
    CHECK_NEW(frame.data[3].b == 0);  // No new head teleports to the start.
    for (const auto t : {875U, 900U, 999U, 1000U}) {
        effect.render(frame.pixels, t);
        expectFixed(frame, 0);
    }
    effect.render(frame.pixels, 1001);
    CHECK_NEW(frame.data[3].b < 3);  // Next entry fades up rather than jumping.
}

void comet_tail_length_changes_the_decay() {
    NewFrame frame;
    CometChase shortTail(1000, 0, 255, 1);
    CometChase longTail(1000, 0, 255, 3);
    shortTail.render(frame.pixels, 500);  // head = 2
    CHECK_NEW(frame.data[1].b == 255);
    CHECK_NEW(frame.data[2].b == 0);
    longTail.render(frame.pixels, 375);  // head = 2 again
    CHECK_NEW(frame.data[2].b > 0);
    CHECK_NEW(frame.data[3].b > 0);
    CometChase zeroTail(1000, 0, 255, 0);
    zeroTail.render(frame.pixels, 500);
    CHECK_NEW(frame.data[1].b == 255);
}

void continuous_effects_are_smooth_and_periodic() {
    NewFrame frame;
    ScannerSweep scan(600, 0, 255, 110);
    CenterOutExplosion expand(700, 0, 255);
    RotatingBeacon rotate(800, 0, 255);
    CometChase chase(1000, 0, 255, 3);
    const Animation* const effects[] = {&scan, &expand, &rotate, &chase};
    constexpr std::uint32_t Cycles[] = {600, 700, 800, 1000};
    for (std::size_t e = 0; e < 4; ++e) {
        effects[e]->render(frame.pixels, 0);
        auto previous = frame.data;
        for (std::uint32_t t = 1; t <= Cycles[e]; ++t) {
            effects[e]->render(frame.pixels, t);
            for (std::size_t i = 0; i < frame.data.size(); ++i) {
                const int delta = static_cast<int>(frame.intensity(i)) -
                    ((i < 4 || i >= 12) ? previous[i].b : previous[i].r);
                CHECK_NEW(delta >= -10 && delta <= 10);
            }
            previous = frame.data;
        }
        for (std::uint32_t t = 0; t < Cycles[e]; t += 17) {
            effects[e]->render(frame.pixels, t);
            const auto expected = frame.data;
            effects[e]->render(frame.pixels, t + Cycles[e]);
            CHECK_NEW(frame.data == expected);
        }
    }
}

void fixed_color_effects_are_synchronized_and_bounded() {
    NewFrame frame;
    ScannerSweep scan(601, 30, 180, 110);
    CenterOutExplosion expand(703, 30, 180);
    RotatingBeacon rotate(803, 30, 180);
    CometChase chase(1003, 30, 180, 3);
    const Animation* const effects[] = {&scan, &expand, &rotate, &chase};
    for (const auto* effect : effects) {
        for (std::uint32_t t = 0; t <= 2100; t += 3) {
            effect->render(frame.pixels, t);
            for (std::size_t i = 0; i < frame.data.size(); ++i) {
                CHECK_NEW(frame.data[i].g == 0);
                CHECK_NEW((i < 4 || i >= 12) ? frame.data[i].r == 0 : frame.data[i].b == 0);
                CHECK_NEW(frame.intensity(i) >= 30 && frame.intensity(i) <= 180);
            }
            for (std::size_t p = 0; p < 4; ++p) {
                // Reversed A/D comet paths; the scanner, beacon and center-out
                // use matching local positions for all physical groups.
                const auto blue = frame.data[p].b;
                const auto red = frame.data[effect == &chase ? 7 - p : 4 + p].r;
                CHECK_NEW(blue == red);
                CHECK_NEW(frame.data[4 + p].r == frame.data[8 + p].r);
                CHECK_NEW(frame.data[p].b == frame.data[12 + p].b);
            }
        }
    }
}

void new_effects_write_complete_frames_with_valid_bounds() {
    const Animation* const effects[] = {
        &tripleFlash, &scanner, &checkerboard, &centerOut, &beacon, &comet};
    for (const auto* effect : effects) {
        for (const auto count : {0U, 1U, 4U, 16U, 24U}) {
            std::array<CRGB, 26> storage{};
            storage[0] = CRGB::Green;
            storage[count + 1] = CRGB::White;
            PixelBuffer target(storage.data() + 1, count);
            target.fill(CRGB::Green);
            effect->render(target, 123);
            CHECK_NEW(storage[0] == CRGB::Green);
            CHECK_NEW(storage[count + 1] == CRGB::White);
            for (std::size_t i = 0; i < count; ++i) {
                CHECK_NEW(storage[i + 1].g == 0);  // Every stale green overwritten.
                if (i >= 16) { CHECK_NEW(storage[i + 1] == CRGB::Black); }
            }
        }
    }
}

void new_effects_handle_invalid_and_degenerate_configuration() {
    NewFrame frame;
    ScannerSweep scan(600, 80, 10, 255);
    CenterOutExplosion expand(700, 80, 80);
    RotatingBeacon rotate(800, 80, 10);
    CometChase chase(1000, 80, 10, 3);
    ScannerSweep zeroScan(0, 80, 255, 110);
    CenterOutExplosion zeroExpand(0, 80, 255);
    RotatingBeacon zeroRotate(0, 80, 255);
    CometChase zeroChase(0, 80, 255, 3);
    const Animation* const constant[] = {
        &scan, &expand, &rotate, &chase, &zeroScan, &zeroExpand, &zeroRotate, &zeroChase};
    for (const auto* effect : constant) {
        for (const auto t : {0U, 1U, 389U, 4000U}) {
            effect->render(frame.pixels, t);
            expectFixed(frame, 80);
        }
    }
    PoliceTripleFlash invalidOn(0, 75, 150, 255);
    PoliceTripleFlash invalidOff(75, 0, 150, 255);
    PoliceTripleFlash invalidPause(75, 75, 0, 255);
    RedBlueCheckerboard invalidChecker(0, 255);
    const Animation* const off[] = {&invalidOn, &invalidOff, &invalidPause, &invalidChecker};
    for (const auto* effect : off) {
        frame.pixels.fill(CRGB::White);
        effect->render(frame.pixels, 999);
        expectFixed(frame, 0);
    }
}

void real_firmware_registers_seven_active_effects_in_order() {
    CHECK_NEW(sizeof(animations) / sizeof(animations[0]) == 7);
    const char* const names[] = {
        "Alternating red and blue", "Alternating police strobes",
        "Synchronized brightness waves", "Police triple flash",
        "Center-out explosion", "Rotating beacon", "Comet chase"};
    for (std::size_t i = 0; i < 7; ++i) {
        CHECK_NEW(std::string(animations[i]->name()) == names[i]);
    }
}

void real_firmware_rotates_and_restarts_every_effect() {
    constexpr auto ActiveCount = sizeof(animations) / sizeof(animations[0]);
    host_clock::nowMs = 0;
    FastLED.showCount = 0;
    setup();
    CHECK_NEW(FastLED.showCount == 1);
    CHECK_NEW(FastLED.brightness == config::GlobalBrightness);
    CHECK_NEW(leds[0] == CRGB::Blue && leds[4] == CRGB::Red);
    for (std::uint32_t interval = 0; interval < 2 * ActiveCount; ++interval) {
        host_clock::nowMs = interval * 4000;
        const auto showsBefore = FastLED.showCount;
        loop();
        CHECK_NEW(scheduler.currentIndex() == interval % ActiveCount);
        NewFrame expected;
        animations[interval % ActiveCount]->render(expected.pixels, 0);
        for (std::size_t i = 0; i < expected.data.size(); ++i) {
            CHECK_NEW(leds[i] == expected.data[i]);
        }
        if (interval != 0) { CHECK_NEW(FastLED.showCount == showsBefore + 1); }
        CHECK_NEW(FastLED.brightness == 50);
        host_clock::nowMs += 3999;
        loop();
        CHECK_NEW(scheduler.currentIndex() == interval % ActiveCount);
    }
    host_clock::nowMs = static_cast<std::uint32_t>(2 * ActiveCount * 4000);
    loop();
    CHECK_NEW(scheduler.currentIndex() == 0);
}

void real_firmware_matches_the_original_three_effects() {
    const Animation* const originals[] = {&alternatingColors, &policeStrobe, &synchronizedWaves};
    for (std::size_t selected = 0; selected < 3; ++selected) {
        const std::uint32_t start = static_cast<std::uint32_t>(selected * 10000);
        scheduler.select(selected, start);
        host_clock::nowMs = start;
        loop();
        for (const auto elapsed : {0U, 25U, 50U, 199U, 500U, 750U, 999U, 3999U}) {
            NewFrame expected;
            originals[selected]->render(expected.pixels, elapsed);
            host_clock::nowMs = start + elapsed;
            loop();
            for (std::size_t i = 0; i < 16; ++i) {
                CHECK_NEW(leds[i] == expected.data[i]);
            }
        }
    }
}

void new_effects_rollover_and_skipped_frames() {
    constexpr auto ActiveCount = sizeof(animations) / sizeof(animations[0]);
    const std::uint32_t start = std::numeric_limits<std::uint32_t>::max() - 100;
    for (std::size_t selected = 3; selected < ActiveCount; ++selected) {
        scheduler.select(selected, start);
        host_clock::nowMs = start;
        loop();
        for (const auto elapsed : {33U, 417U, 1050U, 2099U, 3999U}) {
            NewFrame expected;
            animations[selected]->render(expected.pixels, elapsed);
            host_clock::nowMs = start + elapsed;
            loop();
            for (std::size_t i = 0; i < 16; ++i) {
                CHECK_NEW(leds[i] == expected.data[i]);
            }
        }
        host_clock::nowMs = start + 4000U;
        loop();
        CHECK_NEW(scheduler.currentIndex() == (selected + 1) % ActiveCount);
    }
}

void brightness_cap_remains_separate_from_raw_effect_intensity() {
    NewFrame frame;
    const Animation* const effects[] = {&tripleFlash, &scanner, &checkerboard,
                                        &centerOut, &beacon, &comet};
    constexpr std::uint32_t PeakTimes[] = {0, 0, 0, 175, 0, 125};
    for (std::size_t e = 0; e < 6; ++e) {
        effects[e]->render(frame.pixels, PeakTimes[e]);
        bool fullIntensity = false;
        for (const auto& color : frame.data) {
            fullIntensity = fullIntensity || color.r == 255 || color.b == 255;
        }
        CHECK_NEW(fullIntensity);  // Renderer did not pre-apply global cap.
    }
    host_clock::nowMs = 0;
    setup();
    CHECK_NEW(FastLED.brightness == 50);  // Actual setup retains output cap.
}

void new_effects_repeated_unchanged_frames_skip_show() {
    constexpr auto ActiveCount = sizeof(animations) / sizeof(animations[0]);
    host_clock::nowMs = 0;
    setup();
    for (std::size_t selected = 3; selected < ActiveCount; ++selected) {
        scheduler.select(selected, 0);
        loop();
        const auto before = FastLED.showCount;
        loop();
        CHECK_NEW(FastLED.showCount == before);
    }
}
}  // namespace

int runNewAnimationTests() {
    struct Test { const char* name; void (*run)(); };
    const Test tests[] = {
        {"triple flash exact boundaries and counts", triple_flash_boundaries_and_exact_flash_count},
        {"triple flash timing brightness and wide arithmetic", triple_flash_custom_timing_and_brightness},
        {"scanner smooth motion and reversal", scanner_reverses_and_interpolates},
        {"scanner configurable fading trail", scanner_trail_control_is_visible_and_bounded},
        {"checkerboard exact mapping swap and restart", checkerboard_exact_mapping_and_restart},
        {"center-out expansion contraction and fade", center_out_origin_expansion_contraction_and_fade},
        {"beacon circular direction and wrap", beacon_rotates_forward_and_wraps_circularly},
        {"comet asymmetric tail exit and restart", comet_has_an_asymmetric_tail_and_leaves_before_restart},
        {"comet configurable trail length", comet_tail_length_changes_the_decay},
        {"continuous effects smooth cycle boundaries", continuous_effects_are_smooth_and_periodic},
        {"new effects synchronization colors and bounds", fixed_color_effects_are_synchronized_and_bounded},
        {"new effects complete frames and safe buffer bounds", new_effects_write_complete_frames_with_valid_bounds},
        {"new effects invalid and degenerate inputs", new_effects_handle_invalid_and_degenerate_configuration},
        {"actual firmware seven active effects in order", real_firmware_registers_seven_active_effects_in_order},
        {"actual firmware rotation and restart", real_firmware_rotates_and_restarts_every_effect},
        {"actual firmware original three effects", real_firmware_matches_the_original_three_effects},
        {"new effects rollover and skipped updates", new_effects_rollover_and_skipped_frames},
        {"new effects retain the global brightness cap", brightness_cap_remains_separate_from_raw_effect_intensity},
        {"new effects skip unchanged transmissions", new_effects_repeated_unchanged_frames_skip_show},
    };
    int passed = 0;
    for (const auto& test : tests) {
        try {
            test.run();
            ++passed;
            std::cout << "PASS: " << test.name << '\n';
        } catch (const std::exception& error) {
            std::cerr << "FAIL: " << test.name << ": " << error.what() << '\n';
            return -1;
        }
    }
    return passed;
}
