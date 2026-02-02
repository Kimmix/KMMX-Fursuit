#include "KMMXController.h"
#include <cmath>

void KMMXController::resetIdletime(KMMXController *controller) {
    controller->eyeState.playPrevState();
    controller->mouthState.resetMovingMouth();
    controller->mouthState.setSlowAnimation(false);
    controller->stillTime = 0;
    controller->motionCounter = 0;
    controller->isSleeping = false;
    controller->hornLED.setBrightness(controller->prevHornBright, 5);

    if (enableIdleDebug) {
        Serial.println("[IDLE] Reset - Motion detected, returning to active state");
    }
}

void KMMXController::resetIdletime() {
    if (isSleeping) {
        eyeState.playPrevState();
        mouthState.resetMovingMouth();
        mouthState.setSlowAnimation(false);
    }
    stillTime = 0;
    motionCounter = 0;
    isSleeping = false;
    hornLED.setBrightness(prevHornBright, 5);

    if (enableIdleDebug) {
        Serial.println("[IDLE] Reset - Motion detected, returning to active state");
    }
}

void KMMXController::sleep(KMMXController *controller) {
    controller->prevHornBright = controller->hornLED.getBrightness();
    controller->hornLED.setBrightness(5);
    controller->eyeState.setState(EyeStateEnum::SLEEP);
    controller->mouthState.setSlowAnimation(true);
    controller->isSleeping = true;

    if (enableIdleDebug) {
        Serial.println("[IDLE] Entering sleep mode");
    }
}

void KMMXController::checkIdleAndSleep(KMMXController *controller, unsigned long currentTime) {
    // Only check idle/sleep when in IDLE or SLEEP eye states
    if (controller->eyeState.getState() != EyeStateEnum::IDLE &&
        controller->eyeState.getState() != EyeStateEnum::SLEEP) {
        return;
    }

    const SensorData& current = controller->sensorBuffer[controller->activeBuffer];

    // Initialize baseline on first run
    if (!controller->baselineInitialized) {
        controller->baselineAccel = current;
        controller->baselineInitialized = true;
        controller->stillTime = currentTime;

        if (enableIdleDebug) {
            Serial.printf("[IDLE] Baseline initialized: mag=%.2f m/s²\n", current.accelMagnitude);
        }
        return;
    }

    // Calculate magnitude difference from baseline
    float magnitudeDelta = fabsf(current.accelMagnitude - controller->baselineAccel.accelMagnitude);

    // Select threshold based on current state
    float currentThreshold = controller->isSleeping ? sleepingAccThreshold : idleAccThreshold;

    // Check for motion
    bool motionDetected = (magnitudeDelta > currentThreshold);

    // Debug output with proper rate limiting (once per second)
    static unsigned long lastDebugTime = 0;
    if (enableIdleDebug && (currentTime - lastDebugTime >= 1000)) {
        lastDebugTime = currentTime;
        Serial.printf("[IDLE] Mag: %.2f | Baseline: %.2f | Delta: %.2f | Threshold: %.2f | Motion: %s | Counter: %d\n",
                      current.accelMagnitude,
                      controller->baselineAccel.accelMagnitude,
                      magnitudeDelta,
                      currentThreshold,
                      motionDetected ? "YES" : "NO",
                      controller->motionCounter);
    }

    if (motionDetected) {
        // Motion detected - increment hysteresis counter
        controller->motionCounter++;

        if (enableIdleDebug) {
            Serial.printf("[IDLE] ⚡ MOTION! Mag: %.2f | Baseline: %.2f | Delta: %.2f | Threshold: %.2f | Counter: %d/%d | Sleeping: %s\n",
                          current.accelMagnitude,
                          controller->baselineAccel.accelMagnitude,
                          magnitudeDelta,
                          currentThreshold,
                          controller->motionCounter,
                          motionHysteresisCount,
                          controller->isSleeping ? "YES" : "NO");
        }

        // If sleeping, require sustained motion to wake up (hysteresis)
        if (controller->isSleeping) {
            if (controller->motionCounter >= motionHysteresisCount) {
                resetIdletime(controller);
                // Don't update baseline here - let it update when still in new position
            }
            // Don't reset counter while sleeping - let it accumulate
        }
        // If not sleeping, motion just resets the idle timer (prevents sleep)
        else {
            controller->stillTime = currentTime;  // Reset idle timer
        }
    } else {
        // No motion detected
        // While sleeping, decay counter slowly instead of instant reset
        // This allows motion to accumulate even with gaps between movements
        if (controller->isSleeping) {
            // Decay by 1 every N samples (configurable decay rate)
            static uint8_t decayCounter = 0;
            decayCounter++;
            if (decayCounter >= motionCounterDecayRate && controller->motionCounter > 0) {
                controller->motionCounter--;
                decayCounter = 0;
                if (enableIdleDebug && controller->motionCounter > 0) {
                    Serial.printf("[IDLE] 🔻 Counter decayed to %d/%d\n",
                                  controller->motionCounter, motionHysteresisCount);
                }
            }
        } else {
            // When awake, instant reset
            controller->motionCounter = 0;
        }

        // Update baseline after being still for configured delay (prevents drift during motion)
        // if (controller->stillTime > 0 &&
        //     (currentTime - controller->stillTime >= baselineUpdateDelay) &&
        //     (currentTime - controller->stillTime < baselineUpdateDelay + 100)) {  // Only update once in this window
        //     controller->baselineAccel = current;
        //     if (enableIdleDebug) {
        //         Serial.printf("[IDLE] Baseline updated to %.2f m/s² (still for %.1fs)\n",
        //                       current.accelMagnitude,
        //                       baselineUpdateDelay / 1000.0);
        //     }
        // }

        // Check if should enter sleep
        if (controller->stillTime == 0) {
            controller->stillTime = currentTime;
        } else if (!controller->isSleeping &&
                   (currentTime - controller->stillTime >= idleTimeout)) {
            // Been still long enough - go to sleep
            sleep(controller);
        }
    }
}
