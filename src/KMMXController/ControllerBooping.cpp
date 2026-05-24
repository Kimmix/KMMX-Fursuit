#include "KMMXController.h"

/**
 * Handle boop detection and state transitions.
 *
 * Processes normalized proximity sensor data (0-1023) through the Boop state machine
 * and triggers appropriate visual/audio responses. Called periodically (every 50ms).
 *
 * Sensor data contract:
 * - sensorBuffer[activeBuffer].proximity contains normalized value (0-1023)
 * - Provided by either VL6180X or APDS9930 sensor
 * - Both sensors normalize their output to the same 0-1023 range
 */
void KMMXController::handleBoop() {
    static bool lastIsAngry = false;
    static bool wasInBoopRange = false;
    static bool wasBooped = false;

    if (millis() >= nextBoop) {
        nextBoop = millis() + 50;  // Update every 50ms

        // Process proximity sensor data through boop state machine
        // Input: normalized proximity value (0-1023)
        // Outputs: state flags for rendering appropriate responses
        boop.getBoop(sensorBuffer[activeBuffer].proximity, inBoopRange, isBooping, boopSpeed, isContinuousBoop, isAngry);

        // Handle boop completed (object reached boopMaxThreshold ~900)
        if (isBooping) {
            fxState.setFlyingSpeed(boopSpeed);     // Set heart animation speed based on boop speed
            fxState.setState(FXStateEnum::Heart);   // Show heart effect

            // Only set BOOP state if not already in BOOP state
            // This prevents resetting the animation and causing jerky playback
            if (eyeState.getState() != EyeStateEnum::BOOP) {
                eyeState.setState(EyeStateEnum::BOOP, false, 2500);  // Temporary, 2.5s timeout
            }
            if (mouthState.getState() != MouthStateEnum::BOOP) {
                mouthState.setState(MouthStateEnum::BOOP, false, 700);  // Temporary, 700ms timeout
            }
            resetIdleTime();
            statusLED.setColor(Color::CYAN);
            wasBooped = true;
            wasInBoopRange = false;
        }
        // Handle object in boop range (boopMinThreshold ~100 < value < boopMaxThreshold ~900)
        else if (inBoopRange) {
            if (mouthState.getState() != MouthStateEnum::BOOP) {
                mouthState.setState(MouthStateEnum::BOOP, false, 700);  // Temporary, 700ms timeout
            }
            if (isSleeping) {
                resetIdleTime();  // Wake up from sleep
            }
            statusLED.setColor(Color::LIGHT_PINK);
            wasInBoopRange = true;
        }
        // Handle continuous boop (object held at max threshold)
        else if (isContinuousBoop) {
            // Only set BOOP state if not already in BOOP state
            // This prevents resetting the auto-reset timer
            if (eyeState.getState() != EyeStateEnum::BOOP) {
                eyeState.setState(EyeStateEnum::BOOP, false, 2500);  // Temporary, 2.5s timeout
            }
            statusLED.setColor(Color::PINK);
            wasBooped = true;
            wasInBoopRange = false;
        }
        // Handle angry state (sensor value >= 1023, object too close/touching)
        else if (isAngry && !lastIsAngry) {
            nextBoop = millis() + 1500;  // Delay next boop check during angry animation
            eyeState.setState(EyeStateEnum::ANGRY, false, 1500);      // Temporary, 1.5s timeout
            mouthState.setState(MouthStateEnum::ANGRYBOOP, false, 1500);  // Temporary, 1.5s timeout
            resetIdleTime();
            statusLED.setColor(Color::RED);
            wasBooped = false;
            wasInBoopRange = false;
        }
        // Handle idle state (no active boop conditions)
        else {
            // If we were in range but didn't get booped, show sad eyes
            // (object approached but moved away before completing boop)
            if (wasInBoopRange && !wasBooped) {
                if (eyeState.getState() != EyeStateEnum::SAD) {
                    eyeState.setState(EyeStateEnum::SAD, false, 3000);  // Temporary, 3s timeout
                    statusLED.setColor(Color::BLUE);
                }
            }
            wasInBoopRange = false;
            wasBooped = false;
        }
        lastIsAngry = isAngry;  // Track angry state for edge detection
    }
}
