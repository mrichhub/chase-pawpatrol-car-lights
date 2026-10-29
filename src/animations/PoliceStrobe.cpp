#include "PoliceStrobe.h"
#include "PixelMapping.h"
#include <limits>

namespace {
constexpr std::size_t BluePixels[] = {0, 1, 2, 3, 12, 13, 14, 15};
constexpr std::size_t RedPixels[] = {4, 5, 6, 7, 8, 9, 10, 11};
}  // namespace

bool PoliceStrobe::render(PixelBuffer& pixels, std::uint32_t elapsedMs) const {
    if (colorPhaseMs_ == 0 || onMs_ == 0 || offMs_ == 0 ||
        onMs_ > std::numeric_limits<std::uint32_t>::max() - offMs_) {
        return pixels.fill(CRGB::Black);
    }

    const bool bluePhase = (elapsedMs / colorPhaseMs_) % 2 == 0;
    // Start the ON/OFF clock anew in every color phase, even if phase length
    // is not a multiple of the strobe cycle (or the duty cycle is asymmetric).
    const auto phaseElapsedMs = elapsedMs % colorPhaseMs_;
    const bool on = phaseElapsedMs % (onMs_ + offMs_) < onMs_;

    bool changed = false;
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        CRGB color = CRGB::Black;
        if (on && bluePhase && pixel_mapping::contains(BluePixels, i)) {
            color = CRGB::Blue;
        } else if (on && !bluePhase && pixel_mapping::contains(RedPixels, i)) {
            color = CRGB::Red;
        }
        changed = pixels.set(i, color) || changed;
    }
    return changed;
}
