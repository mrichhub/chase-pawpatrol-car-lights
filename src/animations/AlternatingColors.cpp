#include "AlternatingColors.h"
#include "PixelMapping.h"

namespace {
constexpr std::size_t InitialBluePixels[] = {0, 1, 2, 3, 8, 9, 10, 11};
constexpr std::size_t InitialRedPixels[] = {4, 5, 6, 7, 12, 13, 14, 15};
}  // namespace

bool AlternatingColors::render(PixelBuffer& pixels, std::uint32_t elapsedMs) const {
    // Config.h rejects zero; also fail safely if called with invalid timings
    // by future code that constructs an animation directly.
    if (phaseMs_ == 0) {
        return pixels.fill(CRGB::Black);
    }

    const bool swapped = (elapsedMs / phaseMs_) % 2 != 0;
    bool changed = false;
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        CRGB color = CRGB::Black;  // Additional, unmapped pixels remain OFF.
        if (pixel_mapping::contains(InitialBluePixels, i)) {
            color = swapped ? CRGB::Red : CRGB::Blue;
        } else if (pixel_mapping::contains(InitialRedPixels, i)) {
            color = swapped ? CRGB::Blue : CRGB::Red;
        }
        changed = pixels.set(i, color) || changed;
    }
    return changed;
}
