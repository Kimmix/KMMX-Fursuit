#pragma once
#include <Arduino.h>

/**
 * Time-based animation system that renders frames based on elapsed time
 * instead of sequential frame progression. Supports automatic frame skipping.
 */

// Animation playback mode
enum class AnimationPlayMode {
    ONCE,           // Play once and stop at last frame
    LOOP,           // Loop continuously
    PING_PONG       // Play forward then backward repeatedly
};

// Time-based animation configuration
// Note: Frame skipping is always enabled to maintain timing
// Note: Easing uses a fixed strength of 0.5 when enabled
struct TimeBasedAnimConfig {
    unsigned long durationMs;       // Total duration for one-way playback (ms)
    AnimationPlayMode playMode;     // How the animation should play
    unsigned long pauseAtEndMs;     // Pause duration at end (for PING_PONG)
    unsigned long pauseAtStartMs;   // Pause duration at start (for PING_PONG)
    bool useEasing;                 // Apply easing to frame progression
};

// Time-based animation state
struct TimeBasedAnimState {
    const uint8_t** frames;         // Array of frame pointers
    short frameCount;               // Total number of frames
    short currentFrameIndex;        // Current frame being displayed

    TimeBasedAnimConfig config;     // Animation configuration

    unsigned long animStartTime;    // When current animation cycle started
    unsigned long pauseStartTime;   // When pause started (if paused)
    bool isPaused;                  // Currently in pause state
    bool isReversing;               // Playing in reverse (for PING_PONG)
    bool hasCompleted;              // Animation completed (for ONCE modes)
};

class TimeBasedAnimation {
public:
    // Preset configurations
    static const TimeBasedAnimConfig CONFIG_QUICK_LOOP;      // 200ms loop
    static const TimeBasedAnimConfig CONFIG_SMOOTH_LOOP;     // 500ms smooth loop
    static const TimeBasedAnimConfig CONFIG_SLOW_LOOP;       // 1000ms slow loop
    static const TimeBasedAnimConfig CONFIG_BREATHING;       // 2000ms breathing with pauses
    static const TimeBasedAnimConfig CONFIG_BLINK;           // 150ms quick blink once
    static const TimeBasedAnimConfig CONFIG_TRANSITION;      // 300ms one-shot transition
    static const TimeBasedAnimConfig CONFIG_SMILE_LOOP;      // 500ms smile loop with pauses

    // Blink speed variants (for dynamic blink variance)
    static const TimeBasedAnimConfig CONFIG_BLINK_VERY_FAST;      // 100ms very fast blink
    static const TimeBasedAnimConfig CONFIG_BLINK_VERY_FAST_DBL;  // 100ms very fast (no pause for double blink)
    static const TimeBasedAnimConfig CONFIG_BLINK_FAST;           // 120ms fast blink
    static const TimeBasedAnimConfig CONFIG_BLINK_FAST_DBL;       // 120ms fast (no pause for double blink)
    static const TimeBasedAnimConfig CONFIG_BLINK_NORMAL;         // 150ms normal blink
    static const TimeBasedAnimConfig CONFIG_BLINK_NORMAL_DBL;     // 150ms normal (no pause for double blink)
    static const TimeBasedAnimConfig CONFIG_BLINK_SLOW;           // 200ms slow blink
    static const TimeBasedAnimConfig CONFIG_BLINK_SLOW_DBL;       // 200ms slow (no pause for double blink)
    static const TimeBasedAnimConfig CONFIG_BLINK_VERY_SLOW;      // 250ms very slow blink
    static const TimeBasedAnimConfig CONFIG_BLINK_VERY_SLOW_DBL;  // 250ms very slow (no pause for double blink)

    // Mouth animation presets
    static const TimeBasedAnimConfig CONFIG_WAH;                  // 800ms wah animation
    static const TimeBasedAnimConfig CONFIG_BREATHING_SLOW;       // 3000ms slow breathing

    // Smile transition preset
    static const TimeBasedAnimConfig CONFIG_SMILE_TRANSITION;     // 500ms smile transition

    /**
     * Initialize a time-based animation
     * @param anim Animation state to initialize
     * @param frames Array of frame pointers
     * @param frameCount Number of frames
     * @param config Animation configuration
     */
    static void init(TimeBasedAnimState& anim, const uint8_t** frames, short frameCount, const TimeBasedAnimConfig& config);

    /**
     * Update animation and get current frame to render
     * @param anim Animation state
     * @return Pointer to current frame bitmap
     */
    static const uint8_t* update(TimeBasedAnimState& anim);

    /**
     * Reset animation to beginning
     * @param anim Animation state
     */
    static void reset(TimeBasedAnimState& anim);

    /**
     * Change animation configuration at runtime
     * @param anim Animation state
     * @param config New configuration
     */
    static void setConfig(TimeBasedAnimState& anim, const TimeBasedAnimConfig& config);

    /**
     * Check if animation has completed (for ONCE modes)
     * @param anim Animation state
     * @return true if completed
     */
    static bool isComplete(const TimeBasedAnimState& anim);

    /**
     * Get current progress (0.0 - 1.0)
     * @param anim Animation state
     * @return Progress value
     */
    static float getProgress(const TimeBasedAnimState& anim);

private:
    /**
     * Calculate which frame should be displayed based on elapsed time
     * @param anim Animation state
     * @param elapsedMs Elapsed time in milliseconds
     * @return Frame index to display
     */
    static short calculateFrameIndex(const TimeBasedAnimState& anim, unsigned long elapsedMs);

    /**
     * Apply easing function to linear progress
     * Uses a fixed strength of 0.5 for balanced smoothing
     * @param t Linear progress (0.0 - 1.0)
     * @return Eased progress
     */
    static float applyEasing(float t);
};

