#include "AnimationScheduler.h"

AnimationScheduler::AnimationScheduler(const Animation* const* animations,
                                       std::size_t count,
                                       std::uint32_t rotationMs)
    : animations_(animations), count_(count), rotationMs_(rotationMs) {
    valid_ = animations_ != nullptr && count_ > 0 && rotationMs_ > 0;
    if (valid_) {
        for (std::size_t i = 0; i < count_; ++i) {
            if (animations_[i] == nullptr) {
                valid_ = false;
                break;
            }
        }
    }
}

void AnimationScheduler::begin(std::uint32_t nowMs) {
    select(0, nowMs);
}

bool AnimationScheduler::select(std::size_t index, std::uint32_t nowMs) {
    if (!valid_ || index >= count_) {
        return false;
    }
    currentIndex_ = index;
    activatedAtMs_ = nowMs;
    started_ = true;
    firstFrame_ = true;
    return true;
}

bool AnimationScheduler::next(std::uint32_t nowMs) {
    return select(started_ ? (currentIndex_ + 1) % count_ : 0, nowMs);
}

const char* AnimationScheduler::currentName() const {
    return valid_ && started_ ? animations_[currentIndex_]->name() : "None";
}

bool AnimationScheduler::update(std::uint32_t nowMs, PixelBuffer& pixels) {
    if (!valid_) {
        return false;
    }
    if (!started_) {
        begin(nowMs);
    }

    if (firstFrame_) {
        // A selection request takes effect when its first frame is rendered.
        // Even if update() was delayed after select(), give the new effect
        // its complete interval starting with elapsed zero.
        activatedAtMs_ = nowMs;
    }

    // Unsigned subtraction works through millis() rollover. Never compare
    // against activatedAt + duration, which would break near wraparound.
    std::uint32_t elapsedMs = nowMs - activatedAtMs_;
    if (elapsedMs >= rotationMs_) {
        // Start the next effect NOW at elapsed zero. Do not skip its beginning
        // or replay missed intervals after a delayed loop.
        next(nowMs);
        elapsedMs = 0;
    }

    const bool firstFrame = firstFrame_;
    if (firstFrame) {
        // Clear only RAM, never transmit a separate blackout frame. This also
        // protects transitions if a future animation leaves a pixel unmapped.
        pixels.fill(CRGB::Black);
        firstFrame_ = false;
        elapsedMs = 0;
    }
    const bool changed = animations_[currentIndex_]->render(pixels, elapsedMs);
    return firstFrame || changed;
}
