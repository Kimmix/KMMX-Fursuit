#include "Utils.h"
#include <Arduino.h>

float mapFloat(float x, float inMin, float inMax, float outMin, float outMax) {
    float mappedValue = (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
    return constrain(mappedValue, outMin, outMax);
}

int smoothAccelerometerMovement(float rawValue, float& smoothedValue,
                               float threshold, float maxThreshold,
                               float smoothingFactor, float deadZone,
                               int maxLevel, bool isNegativeDirection) {
    // Apply smoothing filter to reduce jittery movements
    smoothedValue = (smoothingFactor * rawValue) + ((1.0f - smoothingFactor) * smoothedValue);

    // Check if movement exceeds threshold considering direction and dead zone
    bool movementDetected = isNegativeDirection ?
        (smoothedValue < threshold - deadZone) :
        (smoothedValue > threshold + deadZone);

    if (movementDetected) {
        // Calculate normalized level (0.0 to 1.0)
        float normalizedLevel = (smoothedValue - threshold) / (maxThreshold - threshold);
        normalizedLevel = constrain(normalizedLevel, 0.0f, 1.0f);

        // Apply ease-out curve for more natural movement
        float easedLevel = 1.0f - pow(1.0f - normalizedLevel, 2.0f);
        int level = (int)(easedLevel * maxLevel);
        level = constrain(level, 0, maxLevel);

        return level;
    }

    return -1; // No movement detected
}
