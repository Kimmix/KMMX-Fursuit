#pragma once
#include <Arduino.h>
#include "config.h"

/**
 * Boop state machine states.
 *
 * State transitions based on normalized sensor values (0-1023) and
 * thresholds defined in config.h:
 *
 * IDLE              -> No object or value <= boopMinThreshold
 * BOOP_IN_PROGRESS  -> boopMinThreshold < value < boopMaxThreshold
 * BOOP_CONTINUOUS   -> value >= boopMaxThreshold
 * ANGRY             -> value >= 1023 (maximum sensor output)
 */
enum BoopState {
    IDLE,
    BOOP_IN_PROGRESS,
    BOOP_CONTINUOUS,
    ANGRY
};

/**
 * Boop detection and state management.
 *
 * Processes normalized proximity sensor data (0-1023) and manages state
 * transitions. Tracks approach speed based on time elapsed between
 * boopMinThreshold and boopMaxThreshold (see config.h).
 *
 * All sensors must output 0-1023 range (0=far, 1023=touching).
 */
class Boop {
   private:
    BoopState currentBoopState = IDLE;
    unsigned long boopStartTime = 0;

    /** Calculate boop speed from elapsed time (faster = higher 0.0-1.0) */
    float calculateBoopSpeed();

   public:
    /**
     * Process sensor value and update boop state.
     *
     * @param sensorValue Normalized proximity (0-1023)
     * @param isInRange Output: in boop range
     * @param isBoop Output: boop completed
     * @param boopSpeed Output: approach speed (0.0-1.0)
     * @param isContinuous Output: continuous boop active
     * @param isAngry Output: angry state (too close)
     */
    void getBoop(uint16_t& sensorValue, bool& isInRange, bool& isBoop, float& boopSpeed, bool& isContinuous, bool& isAngry);
};
