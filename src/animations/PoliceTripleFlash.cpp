#include "PoliceTripleFlash.h"
#include "EffectMath.h"

bool PoliceTripleFlash::render(PixelBuffer& pixels, std::uint32_t elapsedMs) const {
    if (onMs_ == 0 || offMs_ == 0 || pauseMs_ == 0) {
        return pixels.fill(CRGB::Black);
    }
    const std::uint64_t flashCycle = static_cast<std::uint64_t>(onMs_) + offMs_;
    const std::uint64_t colorDuration = 3ULL * onMs_ + 2ULL * offMs_ + pauseMs_;
    const auto sequenceTime = elapsedMs % (2 * colorDuration);
    const bool blue = sequenceTime < colorDuration;
    const auto localTime = sequenceTime % colorDuration;
    // The first two flashes include normal OFF intervals. The third's OFF
    // interval is the longer pause, replacing rather than adding to normal OFF.
    const bool on = localTime < 2 * flashCycle
                        ? localTime % flashCycle < onMs_
                        : localTime - 2 * flashCycle < onMs_;

    bool changed = false;
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        const bool bluePixel = i < 4 || i >= 12;
        const auto color = on && bluePixel == blue
                               ? effect_math::fixedColor(i, brightness_) : CRGB::Black;
        changed = pixels.set(i, color) || changed;
    }
    return changed;
}
