#include "TimeBasedAnimation.h"
#include <math.h>

// Preset configurations
const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_QUICK_LOOP = {
    .durationMs = 200,
    .playMode = AnimationPlayMode::LOOP,
    .pauseAtEndMs = 0,
    .pauseAtStartMs = 0,
    .useEasing = false
};

const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_SMOOTH_LOOP = {
    .durationMs = 500,
    .playMode = AnimationPlayMode::LOOP,
    .pauseAtEndMs = 0,
    .pauseAtStartMs = 0,
    .useEasing = true
};

const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_SLOW_LOOP = {
    .durationMs = 1000,
    .playMode = AnimationPlayMode::LOOP,
    .pauseAtEndMs = 0,
    .pauseAtStartMs = 0,
    .useEasing = true
};

const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_BREATHING = {
    .durationMs = 2000,
    .playMode = AnimationPlayMode::PING_PONG,
    .pauseAtEndMs = 500,
    .pauseAtStartMs = 800,
    .useEasing = true
};

const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_BLINK = {
    .durationMs = 150,
    .playMode = AnimationPlayMode::ONCE,
    .pauseAtEndMs = 50,
    .pauseAtStartMs = 0,
    .useEasing = false
};

const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_TRANSITION = {
    .durationMs = 300,
    .playMode = AnimationPlayMode::ONCE,
    .pauseAtEndMs = 0,
    .pauseAtStartMs = 0,
    .useEasing = true
};

const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_SMILE_LOOP = {
    .durationMs = 500,
    .playMode = AnimationPlayMode::PING_PONG,
    .pauseAtEndMs = 150,
    .pauseAtStartMs = 150,
    .useEasing = true
};

// Blink speed variants (for dynamic blink variance)
const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_BLINK_VERY_FAST = {
    .durationMs = 100,
    .playMode = AnimationPlayMode::ONCE,
    .pauseAtEndMs = 30,
    .pauseAtStartMs = 0,
    .useEasing = false
};

const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_BLINK_VERY_FAST_DBL = {
    .durationMs = 100,
    .playMode = AnimationPlayMode::ONCE,
    .pauseAtEndMs = 0,  // No pause for double blink
    .pauseAtStartMs = 0,
    .useEasing = false
};

const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_BLINK_FAST = {
    .durationMs = 120,
    .playMode = AnimationPlayMode::ONCE,
    .pauseAtEndMs = 40,
    .pauseAtStartMs = 0,
    .useEasing = false
};

const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_BLINK_FAST_DBL = {
    .durationMs = 120,
    .playMode = AnimationPlayMode::ONCE,
    .pauseAtEndMs = 0,  // No pause for double blink
    .pauseAtStartMs = 0,
    .useEasing = false
};

const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_BLINK_NORMAL = {
    .durationMs = 150,
    .playMode = AnimationPlayMode::ONCE,
    .pauseAtEndMs = 50,
    .pauseAtStartMs = 0,
    .useEasing = false
};

const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_BLINK_NORMAL_DBL = {
    .durationMs = 150,
    .playMode = AnimationPlayMode::ONCE,
    .pauseAtEndMs = 0,  // No pause for double blink
    .pauseAtStartMs = 0,
    .useEasing = false
};

const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_BLINK_SLOW = {
    .durationMs = 200,
    .playMode = AnimationPlayMode::ONCE,
    .pauseAtEndMs = 60,
    .pauseAtStartMs = 0,
    .useEasing = false
};

const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_BLINK_SLOW_DBL = {
    .durationMs = 200,
    .playMode = AnimationPlayMode::ONCE,
    .pauseAtEndMs = 0,  // No pause for double blink
    .pauseAtStartMs = 0,
    .useEasing = false
};

const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_BLINK_VERY_SLOW = {
    .durationMs = 250,
    .playMode = AnimationPlayMode::ONCE,
    .pauseAtEndMs = 80,
    .pauseAtStartMs = 0,
    .useEasing = false
};

const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_BLINK_VERY_SLOW_DBL = {
    .durationMs = 250,
    .playMode = AnimationPlayMode::ONCE,
    .pauseAtEndMs = 0,  // No pause for double blink
    .pauseAtStartMs = 0,
    .useEasing = false
};

// Mouth animation presets
const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_WAH = {
    .durationMs = 800,
    .playMode = AnimationPlayMode::PING_PONG,
    .pauseAtEndMs = 200,
    .pauseAtStartMs = 200,
    .useEasing = true
};

const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_BREATHING_SLOW = {
    .durationMs = 5000,
    .playMode = AnimationPlayMode::PING_PONG,
    .pauseAtEndMs = 1800,
    .pauseAtStartMs = 2400,
    .useEasing = true
};

// Smile transition preset
const TimeBasedAnimConfig TimeBasedAnimation::CONFIG_SMILE_TRANSITION = {
    .durationMs = 500,
    .playMode = AnimationPlayMode::ONCE,
    .pauseAtEndMs = 0,
    .pauseAtStartMs = 0,
    .useEasing = true
};

void TimeBasedAnimation::init(TimeBasedAnimState& anim, const uint8_t** frames, short frameCount, const TimeBasedAnimConfig& config) {
    anim.frames = frames;
    anim.frameCount = frameCount;
    anim.currentFrameIndex = 0;
    anim.config = config;
    anim.animStartTime = millis();
    anim.pauseStartTime = 0;
    anim.isPaused = false;
    anim.isReversing = false;
    anim.hasCompleted = false;
}

void TimeBasedAnimation::reset(TimeBasedAnimState& anim) {
    anim.currentFrameIndex = 0;
    anim.animStartTime = millis();
    anim.pauseStartTime = 0;
    anim.isPaused = false;
    anim.isReversing = false;
    anim.hasCompleted = false;
}

void TimeBasedAnimation::setConfig(TimeBasedAnimState& anim, const TimeBasedAnimConfig& config) {
    anim.config = config;
    reset(anim);
}

bool TimeBasedAnimation::isComplete(const TimeBasedAnimState& anim) {
    return anim.hasCompleted;
}

float TimeBasedAnimation::getProgress(const TimeBasedAnimState& anim) {
    if (anim.frameCount <= 1) return 1.0f;
    return (float)anim.currentFrameIndex / (float)(anim.frameCount - 1);
}

const uint8_t* TimeBasedAnimation::update(TimeBasedAnimState& anim) {
    unsigned long currentTime = millis();

    // Handle completed animations
    if (anim.hasCompleted) {
        return anim.frames[anim.currentFrameIndex];
    }

    // Handle pause state
    if (anim.isPaused) {
        unsigned long pauseDuration = anim.isReversing ? anim.config.pauseAtStartMs : anim.config.pauseAtEndMs;
        if (currentTime - anim.pauseStartTime >= pauseDuration) {
            anim.isPaused = false;
            anim.animStartTime = currentTime;
        } else {
            return anim.frames[anim.currentFrameIndex];
        }
    }

    // Calculate elapsed time
    unsigned long elapsedMs = currentTime - anim.animStartTime;

    // Calculate target frame index
    short targetFrameIndex = calculateFrameIndex(anim, elapsedMs);

    // Frame skipping is always enabled - jump directly to target frame to maintain timing
    anim.currentFrameIndex = targetFrameIndex;

    // Check if we've reached the end of the animation cycle
    bool reachedEnd = false;
    if (anim.isReversing && anim.currentFrameIndex <= 0) {
        anim.currentFrameIndex = 0;
        reachedEnd = true;
    } else if (!anim.isReversing && anim.currentFrameIndex >= anim.frameCount - 1) {
        anim.currentFrameIndex = anim.frameCount - 1;
        reachedEnd = true;
    }

    // Handle end-of-cycle behavior
    if (reachedEnd) {
        switch (anim.config.playMode) {
            case AnimationPlayMode::ONCE:
                anim.hasCompleted = true;
                break;

            case AnimationPlayMode::LOOP:
                anim.currentFrameIndex = 0;
                anim.animStartTime = currentTime;
                break;

            case AnimationPlayMode::PING_PONG:
                anim.isReversing = !anim.isReversing;
                anim.isPaused = true;
                anim.pauseStartTime = currentTime;
                break;
        }
    }

    return anim.frames[anim.currentFrameIndex];
}

short TimeBasedAnimation::calculateFrameIndex(const TimeBasedAnimState& anim, unsigned long elapsedMs) {
    if (anim.frameCount <= 1) return 0;

    // Calculate linear progress (0.0 - 1.0)
    float progress = (float)elapsedMs / (float)anim.config.durationMs;

    // Clamp to valid range
    if (progress > 1.0f) progress = 1.0f;
    if (progress < 0.0f) progress = 0.0f;

    // Apply easing if enabled (uses fixed strength of 0.5)
    if (anim.config.useEasing) {
        progress = applyEasing(progress);
    }

    // Convert progress to frame index
    short frameIndex;
    if (anim.isReversing) {
        frameIndex = (short)((1.0f - progress) * (anim.frameCount - 1));
    } else {
        frameIndex = (short)(progress * (anim.frameCount - 1));
    }

    // Clamp to valid frame range
    if (frameIndex < 0) frameIndex = 0;
    if (frameIndex >= anim.frameCount) frameIndex = anim.frameCount - 1;

    return frameIndex;
}

float TimeBasedAnimation::applyEasing(float t) {
    // Ease in-out cubic with fixed strength of 0.5
    // Provides a balanced smooth animation curve

    float eased;
    if (t < 0.5f) {
        eased = 4.0f * t * t * t;
    } else {
        float f = (2.0f * t - 2.0f);
        eased = 0.5f * f * f * f + 1.0f;
    }

    // Blend between linear and eased (50/50 blend)
    return t * 0.5f + eased * 0.5f;
}

