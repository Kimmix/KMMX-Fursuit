#pragma once
#include <Arduino.h>
#include "config.h"

/**
 * Boop state machine states.
 *
 * State transitions based on normalized sensor values (0-1023):
 * - IDLE: No object detected or object too far
 * - BOOP_IN_PROGRESS: Object in range (boopMinThreshold < value < boopMaxThreshold)
 * - BOOP_CONTINUOUS: Object at max threshold (value >= boopMaxThreshold)
 * - ANGRY: Object extremely close (value >= 1023)
 *
 * Threshold values from config.h:
 * - boopMinThreshold: 100 (minimum proximity to activate)
 * - boopMaxThreshold: 900 (maximum range for full boop)
 * - 1023: Maximum sensor value (triggers ANGRY state)
 */
enum BoopState {
    IDLE,              // No boop interaction
    BOOP_IN_PROGRESS,  // Object approaching, in valid boop range
    BOOP_CONTINUOUS,   // Object at max threshold, continuous boop
    ANGRY              // Object too close/touching sensor
};

/**
 * Boop detection and state management.
 *
 * This class processes normalized proximity sensor data (0-1023) and manages
 * boop state transitions. It tracks approach speed and provides state flags
 * for rendering appropriate responses.
 *
 * Sensor normalization contract:
 * - All proximity sensors MUST normalize output to 0-1023 range
 * - 0 = no object detected or too far
 * - Higher values = closer proximity
 * - 1023 = extremely close/touching (triggers ANGRY state)
 */
class Boop {
   private:
    BoopState currentBoopState = IDLE;
    unsigned long boopStartTime = 0;

    /**
     * Calculate boop speed based on elapsed time.
     *
     * @return Speed value 0.0-1.0 (faster approach = higher value)
     */
    float calculateBoopSpeed();

   public:
    /**
     * Process sensor value and update boop state.
     *
     * @param sensorValue Normalized proximity value (0-1023)
     * @param isInRange Output: true if object in boop range
     * @param isBoop Output: true if boop completed
     * @param boopSpeed Output: speed of boop (0.0-1.0)
     * @param isContinuous Output: true if continuous boop active
     * @param isAngry Output: true if object too close (angry state)
     */
    void getBoop(uint16_t& sensorValue, bool& isInRange, bool& isBoop, float& boopSpeed, bool& isContinuous, bool& isAngry);
};
