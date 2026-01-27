#include "AnimationHelper.h"

// Preset timing profiles
const AnimationTiming AnimationHelper::TIMING_BOUNCY = {
    .startDelay = 6,
    .midDelay = 3,
    .endDelay = 5,
    .startThreshold = 30,
    .endThreshold = 70,
    .peakPauseMin = 20,
    .peakPauseMax = 40,
    .restPauseMin = 30,
    .restPauseMax = 50,
    .randomVariance = 7,
    .useEasing = true
};

const AnimationTiming AnimationHelper::TIMING_SMOOTH = {
    .startDelay = 4,
    .midDelay = 4,
    .endDelay = 4,
    .startThreshold = 30,
    .endThreshold = 70,
    .peakPauseMin = 18,
    .peakPauseMax = 32,
    .restPauseMin = 28,
    .restPauseMax = 48,
    .randomVariance = 5,
    .useEasing = false
};

const AnimationTiming AnimationHelper::TIMING_SNAPPY = {
    .startDelay = 3,
    .midDelay = 2,
    .endDelay = 3,
    .startThreshold = 20,
    .endThreshold = 80,
    .peakPauseMin = 15,
    .peakPauseMax = 25,
    .restPauseMin = 25,
    .restPauseMax = 40,
    .randomVariance = 9,
    .useEasing = false
};

const AnimationTiming AnimationHelper::TIMING_ELASTIC = {
    .startDelay = 7,
    .midDelay = 2,
    .endDelay = 7,
    .startThreshold = 25,
    .endThreshold = 75,
    .peakPauseMin = 28,
    .peakPauseMax = 48,
    .restPauseMin = 45,
    .restPauseMax = 75,
    .randomVariance = 14,
    .useEasing = true
};

const AnimationTiming AnimationHelper::TIMING_BREATHING = {
    .startDelay = 60,
    .midDelay = 60,
    .endDelay = 60,
    .startThreshold = 40,
    .endThreshold = 80,
    .peakPauseMin = 300,
    .peakPauseMax = 550,
    .restPauseMin = 700,
    .restPauseMax = 1100,
    .randomVariance = 15,
    .useEasing = true
};

const AnimationTiming AnimationHelper::TIMING_BREATHING_SLOW = {
    .startDelay = 250,
    .midDelay = 250,
    .endDelay = 250,
    .startThreshold = 40,
    .endThreshold = 80,
    .peakPauseMin = 800,
    .peakPauseMax = 1200,
    .restPauseMin = 1500,
    .restPauseMax = 2500,
    .randomVariance = 25,
    .useEasing = true
};

void AnimationHelper::initAnimation(AnimationState& anim, const uint8_t** frames, short length, const AnimationTiming& timing) {
    anim.frames = frames;
    anim.length = length;
    anim.index = 0;
    anim.increasing = true;
    anim.interval = 0;
    anim.timing = timing;
}

void AnimationHelper::setTiming(AnimationState& anim, const AnimationTiming& timing) {
    anim.timing = timing;
}

int AnimationHelper::calculateDelay(const AnimationState& anim) {
    float progress = anim.index / (float)(anim.length - 1);
    int progressPercent = (int)(progress * 100);

    int baseDelay;
    if (anim.increasing) {
        // Forward direction: use start/mid/end delays
        if (progressPercent < anim.timing.startThreshold) {
            baseDelay = anim.timing.startDelay;
        } else if (progressPercent < anim.timing.endThreshold) {
            baseDelay = anim.timing.midDelay;
        } else {
            baseDelay = anim.timing.endDelay;
        }
    } else {
        // Reverse direction: mirror the timing
        if (progressPercent > anim.timing.endThreshold) {
            baseDelay = anim.timing.endDelay;
        } else if (progressPercent > anim.timing.startThreshold) {
            baseDelay = anim.timing.midDelay;
        } else {
            baseDelay = anim.timing.startDelay;
        }
    }

    // Add randomness
    int variance = anim.timing.randomVariance;
    int randomVariance = (esp_random() % (variance * 2 + 1)) - variance;

    return baseDelay + randomVariance;
}

const uint8_t* AnimationHelper::updateAnimation(AnimationState& anim) {
    // Update animation timing
    if (millis() >= anim.interval) {
        int delay = calculateDelay(anim);
        anim.interval = millis() + delay;

        // Update index
        if (anim.increasing) {
            anim.index++;
            if (anim.index >= anim.length - 1) {
                anim.index = anim.length - 1;
                anim.increasing = false;
                // Pause at peak
                int pauseRange = anim.timing.peakPauseMax - anim.timing.peakPauseMin;
                anim.interval = millis() + anim.timing.peakPauseMin + (esp_random() % pauseRange);
            }
        } else {
            anim.index--;
            if (anim.index <= 0) {
                anim.index = 0;
                anim.increasing = true;
                // Pause at rest
                int pauseRange = anim.timing.restPauseMax - anim.timing.restPauseMin;
                anim.interval = millis() + anim.timing.restPauseMin + (esp_random() % pauseRange);
            }
        }
    }

    // Return current frame
    return anim.frames[anim.index];
}

float AnimationHelper::easeInOutBack(float t) {
    const float c1 = 1.70158;
    const float c2 = c1 * 1.525;
    return t < 0.5
        ? (pow(2 * t, 2) * ((c2 + 1) * 2 * t - c2)) / 2
        : (pow(2 * t - 2, 2) * ((c2 + 1) * (t * 2 - 2) + c2) + 2) / 2;
}

float AnimationHelper::easeInOutElastic(float t) {
    const float c5 = (2 * PI) / 4.5;
    if (t == 0.0f || t == 1.0f) return t;

    return t < 0.5
        ? -(pow(2, 20 * t - 10) * sin((20 * t - 11.125) * c5)) / 2
        : (pow(2, -20 * t + 10) * sin((20 * t - 11.125) * c5)) / 2 + 1;
}

float AnimationHelper::easeInOutBounce(float t) {
    auto bounceOut = [](float x) -> float {
        const float n1 = 7.5625;
        const float d1 = 2.75;
        if (x < 1 / d1) {
            return n1 * x * x;
        } else if (x < 2 / d1) {
            return n1 * (x -= 1.5 / d1) * x + 0.75;
        } else if (x < 2.5 / d1) {
            return n1 * (x -= 2.25 / d1) * x + 0.9375;
        } else {
            return n1 * (x -= 2.625 / d1) * x + 0.984375;
        }
    };

    return t < 0.5
        ? (1 - bounceOut(1 - 2 * t)) / 2
        : (1 + bounceOut(2 * t - 1)) / 2;
}
