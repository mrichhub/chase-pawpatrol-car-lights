#include "RedBlueCheckerboard.h"
#include "PixelMapping.h"

namespace {
constexpr std::size_t InitialRedPixels[] = {0, 2, 5, 7, 8, 10, 13, 15};
}  // namespace

bool RedBlueCheckerboard::render(PixelBuffer& pixels, std::uint32_t elapsedMs) const {
    if (phaseMs_ == 0) {
        return pixels.fill(CRGB::Black);
    }
    const bool swapped = (elapsedMs / phaseMs_) % 2 != 0;
    bool changed = false;
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        CRGB color = CRGB::Black;
        if (i < 16) {
            const bool red = pixel_mapping::contains(InitialRedPixels, i) != swapped;
            color = red ? CRGB(brightness_, 0, 0) : CRGB(0, 0, brightness_);
        }
        changed = pixels.set(i, color) || changed;
    }
    return changed;
}
