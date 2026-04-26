#pragma once

#include <Arduino.h>

/**
 * @brief True FPS counter that measures actual frame timing
 *
 * This class tracks:
 * - Actual FPS (frames per second based on real timing)
 * - Target FPS (configured refresh rate)
 */
class FPSCounter {
private:
    // Frame timing
    unsigned long lastFrameTime = 0;

    // FPS calculation (1 second rolling average)
    static constexpr int FPS_SAMPLE_COUNT = 60;
    unsigned long frameTimes[FPS_SAMPLE_COUNT] = {0};
    int frameTimeIndex = 0;
    int validSamples = 0;

    // Calculated values
    float currentFPS = 0.0f;
    int targetFPS = 0;

    // Update interval for FPS calculation
    unsigned long lastFPSUpdate = 0;
    static constexpr unsigned long FPS_UPDATE_INTERVAL = 250;  // Update every 250ms

public:
    /**
     * @brief Constructor
     */
    FPSCounter() = default;

    /**
     * @brief Set the target FPS (from display refresh rate)
     * @param fps Target frames per second
     */
    void setTargetFPS(int fps) {
        targetFPS = fps;
    }

    /**
     * @brief Call this once per frame to track FPS
     */
    void update() {
        unsigned long currentTime = micros();

        if (lastFrameTime > 0) {
            unsigned long frameTime = currentTime - lastFrameTime;

            // Store frame time in circular buffer
            frameTimes[frameTimeIndex] = frameTime;
            frameTimeIndex = (frameTimeIndex + 1) % FPS_SAMPLE_COUNT;
            if (validSamples < FPS_SAMPLE_COUNT) {
                validSamples++;
            }

            // Calculate FPS periodically
            if (currentTime - lastFPSUpdate >= FPS_UPDATE_INTERVAL * 1000) {
                calculateFPS();
                lastFPSUpdate = currentTime;
            }
        }

        lastFrameTime = currentTime;
    }

    /**
     * @brief Get the current calculated FPS
     * @return Current FPS as float
     */
    float getFPS() const {
        return currentFPS;
    }

    /**
     * @brief Get the target FPS
     * @return Target FPS
     */
    int getTargetFPS() const {
        return targetFPS;
    }

private:
    /**
     * @brief Calculate FPS from stored frame times
     */
    void calculateFPS() {
        if (validSamples == 0) {
            currentFPS = 0.0f;
            return;
        }

        // Calculate average frame time
        unsigned long totalTime = 0;
        for (int i = 0; i < validSamples; i++) {
            totalTime += frameTimes[i];
        }

        unsigned long avgFrameTime = totalTime / validSamples;

        // Convert to FPS (frame time is in microseconds)
        if (avgFrameTime > 0) {
            currentFPS = 1000000.0f / avgFrameTime;
        } else {
            currentFPS = 0.0f;
        }
    }
};
