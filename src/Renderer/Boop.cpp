#include "Boop.h"
#include "Utils/Utils.h"  // Use optimized utility functions

/**
 * Calculate boop speed based on time elapsed since object entered range.
 *
 * Faster approach (less elapsed time) = higher speed value.
 * Used to determine animation intensity based on boop speed.
 *
 * @return Speed value 0.0-1.0
 */
float Boop::calculateBoopSpeed() {
    unsigned long elapsedTime = millis() - boopStartTime;
    // Use optimized mapFloat for better performance
    // Shorter time = faster speed = higher value
    float speed = mapFloat(elapsedTime, 100, boopMaxDuration, 0, 100);
    return speed / 100;
}

/**
 * Process sensor value and update boop state machine.
 *
 * State transitions:
 * IDLE -> BOOP_IN_PROGRESS: When boopMinThreshold < value < boopMaxThreshold
 * IDLE -> ANGRY: When value >= 1023 (extremely close/touching)
 * BOOP_IN_PROGRESS -> BOOP_CONTINUOUS: When value >= boopMaxThreshold
 * BOOP_IN_PROGRESS -> IDLE: When value < boopMinThreshold (object moved away)
 * BOOP_CONTINUOUS -> IDLE: When value < boopMaxThreshold
 * ANGRY -> IDLE: When value < boopMaxThreshold
 *
 * Sensor normalization requirement:
 * - Both VL6180X and APDS9930 normalize their output to 0-1023
 * - VL6180X: inverts distance (closer = higher value) and maps to 0-1023
 * - APDS9930: normalizes raw proximity with auto-calibration and maps to 0-1023
 * - Both use median filtering to reduce noise
 */
void Boop::getBoop(uint16_t& sensorValue, bool& isInRange, bool& isBoop, float& boopSpeed, bool& isContinuous, bool& isAngry) {
    // Reset all output flags
    isInRange = false;
    isBoop = false;
    isContinuous = false;
    isAngry = false;

    switch (currentBoopState) {
        case IDLE:
            // Check if object enters boop range (between min and max thresholds)
            if (sensorValue > boopMinThreshold && sensorValue < boopMaxThreshold) {
                currentBoopState = BOOP_IN_PROGRESS;
                boopStartTime = millis();  // Start timing for speed calculation
                isInRange = true;
            }
            // Check if object is extremely close (touching sensor)
            else if (sensorValue >= 1023) {
                currentBoopState = ANGRY;
                isAngry = true;
            }
            break;

        case BOOP_IN_PROGRESS:
            isInRange = true;

            // Check if object reached max threshold (full boop)
            if (sensorValue >= boopMaxThreshold) {
                boopSpeed = calculateBoopSpeed();  // Calculate speed based on approach time
                if (boopSpeed > 0.0) {
                    isBoop = true;
                    currentBoopState = BOOP_CONTINUOUS;
                    isContinuous = true;
                } else {
                    // Invalid speed, reset to idle
                    currentBoopState = IDLE;
                }
            }
            // Check if object moved away before completing boop
            else if (sensorValue < boopMinThreshold) {
                currentBoopState = IDLE;
            }
            break;

        case BOOP_CONTINUOUS:
            isContinuous = true;

            // Exit continuous boop when object moves away
            if (sensorValue < boopMaxThreshold) {
                currentBoopState = IDLE;
            }
            break;

        case ANGRY:
            isAngry = true;

            // Exit angry state when object moves away
            if (sensorValue < boopMaxThreshold) {
                currentBoopState = IDLE;
            }
            break;
    }
}