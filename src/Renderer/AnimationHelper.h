#pragma once
#include <Arduino.h>

// Timing profile for animation phases
struct AnimationTiming {
    int startDelay;      // Delay at start of animation
    int midDelay;        // Delay in middle of animation
    int endDelay;        // Delay at end of animation
    int startThreshold;  // Progress threshold for start phase (0-100)
    int endThreshold;    // Progress threshold for end phase (0-100)
    int peakPauseMin;    // Minimum pause at peak (ms)
    int peakPauseMax;    // Maximum pause at peak (ms)
    int restPauseMin;    // Minimum pause at rest (ms)
    int restPauseMax;    // Maximum pause at rest (ms)
    int randomVariance;  // Random timing variance (+/- ms)
    bool useEasing;      // Apply easing function
};

// Generic animation state
struct AnimationState {
    const uint8_t** frames;     // Array of frame pointers
    short length;               // Number of frames
    short index;                // Current frame index
    bool increasing;            // Direction of playback
    unsigned long interval;     // Next update time
    AnimationTiming timing;     // Timing configuration
};

class AnimationHelper {
   public:
    // Preset timing profiles
    static const AnimationTiming TIMING_BOUNCY;    // Bouncy with easing
    static const AnimationTiming TIMING_SMOOTH;    // Smooth consistent speed
    static const AnimationTiming TIMING_SNAPPY;    // Fast and snappy
    static const AnimationTiming TIMING_ELASTIC;   // Elastic bounce effect
    static const AnimationTiming TIMING_BREATHING; // Natural breathing rhythm
    static const AnimationTiming TIMING_BREATHING_SLOW; // Slow, relaxed breathing

    // Initialize an animation with frames and timing
    static void initAnimation(AnimationState& anim, const uint8_t** frames, short length, const AnimationTiming& timing);

    // Change timing profile at runtime
    static void setTiming(AnimationState& anim, const AnimationTiming& timing);

    // Update animation state and return current frame
    static const uint8_t* updateAnimation(AnimationState& anim);

    // Calculate delay based on animation state and timing
    static int calculateDelay(const AnimationState& anim);

    // Easing functions
    static float easeInOutBack(float t);
    static float easeInOutElastic(float t);
    static float easeInOutBounce(float t);
};
