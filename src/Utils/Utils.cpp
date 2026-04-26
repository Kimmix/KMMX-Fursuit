#include "Utils.h"
#include <Arduino.h>

/**
 * Optimized float mapping function for ESP32.
 * Pre-calculates ranges to avoid redundant subtraction operations.
 */
float mapFloat(float x, float inMin, float inMax, float outMin, float outMax) {
    // Pre-calculate ranges to avoid redundant operations
    float inRange = inMax - inMin;
    float outRange = outMax - outMin;

    // Compute mapped value: (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin
    float mappedValue = ((x - inMin) * outRange) / inRange + outMin;

    // Clamp to output range (faster than constrain for simple min/max)
    if (mappedValue < outMin) return outMin;
    if (mappedValue > outMax) return outMax;
    return mappedValue;
}

/**
 * Optimized accelerometer smoothing with ESP32-friendly operations.
 * Uses fast clamping and reduces redundant calculations.
 */
int smoothAccelerometerMovement(float rawValue, float& smoothedValue,
                               float threshold, float maxThreshold,
                               float smoothingFactor, float deadZone,
                               int maxLevel, bool isNegativeDirection) {
    // Apply exponential smoothing filter (optimized: pre-calculate 1-alpha)
    float invSmoothingFactor = 1.0f - smoothingFactor;
    smoothedValue = (smoothingFactor * rawValue) + (invSmoothingFactor * smoothedValue);

    // Pre-calculate threshold with dead zone to avoid redundant operations
    float effectiveThreshold = isNegativeDirection ? (threshold - deadZone) : (threshold + deadZone);

    // Check if movement exceeds threshold considering direction
    bool movementDetected = isNegativeDirection ?
        (smoothedValue < effectiveThreshold) :
        (smoothedValue > effectiveThreshold);

    if (movementDetected) {
        // Calculate normalized level (0.0 to 1.0) - pre-calculate range
        float range = maxThreshold - threshold;
        float normalizedLevel = (smoothedValue - threshold) / range;

        // Fast clamp instead of constrain()
        if (normalizedLevel < 0.0f) normalizedLevel = 0.0f;
        if (normalizedLevel > 1.0f) normalizedLevel = 1.0f;

        // Apply ease-out curve for more natural movement (x² is faster than pow)
        float complement = 1.0f - normalizedLevel;
        float easedLevel = 1.0f - (complement * complement);

        // Convert to integer level with fast clamping
        int level = (int)(easedLevel * maxLevel);
        if (level < 0) level = 0;
        if (level > maxLevel) level = maxLevel;

        return level;
    }

    return -1; // No movement detected
}
