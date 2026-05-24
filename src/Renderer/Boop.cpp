#include "Boop.h"
#include "Utils/Utils.h"  // Use optimized utility functions

/**
 * Calculate boop speed from elapsed time (faster approach = higher speed).
 * Maps time between 100ms and boopMaxDuration to 0.0-1.0 for animation intensity.
 */
float Boop::calculateBoopSpeed() {
    unsigned long elapsedTime = millis() - boopStartTime;
    float speed = mapFloat(elapsedTime, 100, boopMaxDuration, 0, 100);
    return speed / 100;
}

/**
 * Process sensor value and update boop state machine.
 *
 * State transitions (thresholds from config.h):
 * - IDLE -> BOOP_IN_PROGRESS: boopMinThreshold < value < boopMaxThreshold
 * - IDLE -> ANGRY: value >= 1023
 * - BOOP_IN_PROGRESS -> BOOP_CONTINUOUS: value >= boopMaxThreshold
 * - BOOP_IN_PROGRESS -> IDLE: value < boopMinThreshold
 * - BOOP_CONTINUOUS -> IDLE: value < boopMaxThreshold
 * - ANGRY -> IDLE: value < boopMaxThreshold
 */
void Boop::getBoop(uint16_t& sensorValue, bool& isInRange, bool& isBoop, float& boopSpeed, bool& isContinuous, bool& isAngry) {
    isInRange = false;
    isBoop = false;
    isContinuous = false;
    isAngry = false;

    switch (currentBoopState) {
        case IDLE:
            if (sensorValue > boopMinThreshold && sensorValue < boopMaxThreshold) {
                currentBoopState = BOOP_IN_PROGRESS;
                boopStartTime = millis();
                isInRange = true;
            } else if (sensorValue >= 1023) {
                currentBoopState = ANGRY;
                isAngry = true;
            }
            break;

        case BOOP_IN_PROGRESS:
            isInRange = true;
            if (sensorValue >= boopMaxThreshold) {
                boopSpeed = calculateBoopSpeed();
                if (boopSpeed > 0.0) {
                    isBoop = true;
                    currentBoopState = BOOP_CONTINUOUS;
                    isContinuous = true;
                } else {
                    currentBoopState = IDLE;
                }
            } else if (sensorValue < boopMinThreshold) {
                currentBoopState = IDLE;
            }
            break;

        case BOOP_CONTINUOUS:
            isContinuous = true;
            if (sensorValue < boopMaxThreshold) {
                currentBoopState = IDLE;
            }
            break;

        case ANGRY:
            isAngry = true;
            if (sensorValue < boopMaxThreshold) {
                currentBoopState = IDLE;
            }
            break;
    }
}