#include "KMMXController.h"
#include <cmath>

/**
 * Main motion detection dispatcher - called from sensor task
 * Checks all motion features in priority order
 */
void KMMXController::checkMotionFeatures(KMMXController *controller) {
    // Skip motion detection during startup delay (prevents false triggers during sensor initialization)
    if (millis() < controller->motionDetectionStartTime) {
        return;
    }

    // Log when motion detection becomes active (once)
    static bool motionDetectionStarted = false;
    if (!motionDetectionStarted && enableMotionDebug) {
        motionDetectionStarted = true;
        Serial.println("[MOTION] Motion detection is now active");
    }

    // Skip motion detection if in high-priority states
    EyeStateEnum currentEyeState = controller->eyeState.getState();

    // Don't interfere with boop, manual states, or transitions
    if (currentEyeState == EyeStateEnum::BOOP ||
        currentEyeState == EyeStateEnum::ANGRY ||
        currentEyeState == EyeStateEnum::CRY) {
        return;  // Skip motion detection
    }

    // Get current sensor data from active buffer
    const SensorData& current = controller->sensorBuffer[controller->activeBuffer];

    // Check in priority order (first match wins)
    // Shake detection removed
    // Bounce detection removed
    // Spin detection removed

    if (enableTiltDetection) {
        controller->detectTilt(current);
        if (controller->tiltDetector.isTilted) return;
    }

    if (enablePettingDetection) {
        controller->detectPetting(current);
    }
}

// Shake Detection - REMOVED

/**
 * Tilt Detection - Detects sustained head tilt for curious/confused expressions
 * Uses accelerometer axes to detect tilt angle and direction
 */
void KMMXController::detectTilt(const SensorData& current) {
    unsigned long currentTime = millis();

    // Calculate tilt angles from accelerometer data
    float tiltX = current.accelX;  // Forward/back tilt
    float tiltZ = current.accelZ;  // Left/right tilt

    // Check for significant tilt on each axis
    bool isTiltedForwardBack = fabsf(tiltX) > tiltThreshold;
    bool isTiltedLeftRight = fabsf(tiltZ) > tiltThreshold;

    // Check for neutral position (both axes below neutral threshold)
    bool isNeutral = fabsf(tiltX) < tiltNeutralThreshold &&
                     fabsf(tiltZ) < tiltNeutralThreshold;

    // Debounce - prevent rapid state changes
    if (currentTime - tiltDetector.lastTiltChangeTime < tiltDebounceTime) {
        return;
    }

    // ========== STATE 1: Currently Tilted ==========
    if (tiltDetector.isTilted) {
        // STRICT LOCK: If in Forward/Back mode, block Left/Right completely until neutral
        if (!tiltDetector.isLeftRight) {
            // Currently in Forward/Back mode
            if (isTiltedLeftRight && !isTiltedForwardBack) {
                // Trying to switch to Left/Right - BLOCK IT
                if (enableMotionDebug) {
                    Serial.println("[TILT] Left/Right blocked - Forward/Back is active (return to neutral first)");
                }
                return;
            }

            // Check if still tilted forward/back or returning to neutral
            if (isNeutral) {
                // Return to neutral from Forward/Back
                tiltDetector.isTilted = false;
                tiltDetector.tiltStartTime = 0;
                tiltDetector.lastTiltChangeTime = currentTime;
                tiltDetector.lastForwardBackTime = currentTime;  // Track forward/back time
                tiltDetector.lastNeutralReturnTime = currentTime;  // Track neutral return for petting cooldown

                eyeState.setState(tiltDetector.previousEyeState);
                mouthState.setState(tiltDetector.previousMouthState);

                if (enableMotionDebug) {
                    Serial.println("[TILT] Returned to neutral (was Forward/Back)");
                }
            }
            // If still tilted forward/back, maintain current state (no action needed)
            return;
        } else {
            // Currently in Left/Right mode
            // Check if switching to Forward/Back (with cooldown)
            if (isTiltedForwardBack) {
                // Apply cooldown when switching from Left/Right to Forward/Back
                unsigned long timeSinceLastChange = currentTime - tiltDetector.lastTiltChangeTime;
                if (timeSinceLastChange < tiltDirectionChangeCooldown) {
                    if (enableMotionDebug) {
                        Serial.printf("[TILT] Direction change blocked - cooldown active (%lu ms / %u ms)\n",
                                      timeSinceLastChange, tiltDirectionChangeCooldown);
                    }
                    return;
                }

                // Cooldown expired - allow switch to Forward/Back
                tiltDetector.isLeftRight = false;
                tiltDetector.lastTiltChangeTime = currentTime;
                tiltDetector.lastForwardBackTime = currentTime;

                Serial.printf("[TILT] ✓ Direction changed! Angle: %.2f, Direction: Forward/Back\n", tiltX);
                triggerTiltResponse(tiltX, false);
                return;
            }

            // Check if returning to neutral
            if (isNeutral) {
                // Return to neutral from Left/Right
                tiltDetector.isTilted = false;
                tiltDetector.tiltStartTime = 0;
                tiltDetector.lastTiltChangeTime = currentTime;
                tiltDetector.lastNeutralReturnTime = currentTime;  // Track neutral return for petting cooldown

                eyeState.setState(tiltDetector.previousEyeState);
                mouthState.setState(tiltDetector.previousMouthState);

                if (enableMotionDebug) {
                    Serial.println("[TILT] Returned to neutral (was Left/Right)");
                }
            }
            // If still tilted left/right, maintain current state (no action needed)
            return;
        }
    }

    // ========== STATE 2: Not Currently Tilted ==========
    // Check if we should start tracking or trigger a new tilt

    if (isNeutral) {
        // In neutral - reset any tracking
        if (tiltDetector.tiltStartTime != 0) {
            tiltDetector.tiltStartTime = 0;  // Reset tracking silently
        }
        return;
    }

    // Detect which direction to track (Forward/Back has priority)
    bool trackForwardBack = isTiltedForwardBack;
    bool trackLeftRight = isTiltedLeftRight && !isTiltedForwardBack;

    if (!trackForwardBack && !trackLeftRight) {
        return;  // No significant tilt detected
    }

    if (tiltDetector.tiltStartTime == 0) {
        // First time detecting tilt - start tracking
        tiltDetector.tiltStartTime = currentTime;
        tiltDetector.tiltAngleX = tiltX;
        tiltDetector.tiltAngleZ = tiltZ;
        return;
    }

    // Already tracking - check if sustained long enough
    if (currentTime - tiltDetector.tiltStartTime >= tiltSustainTime) {
        // Trigger tilt response
        tiltDetector.isTilted = true;
        tiltDetector.isLeftRight = trackLeftRight;
        tiltDetector.previousEyeState = eyeState.getState();
        tiltDetector.previousMouthState = mouthState.getState();
        tiltDetector.lastTiltChangeTime = currentTime;

        // Track forward/back time for petting cooldown
        if (trackForwardBack) {
            tiltDetector.lastForwardBackTime = currentTime;
        }

        float angle = trackLeftRight ? tiltZ : tiltX;
        const char* direction = trackLeftRight ? "Left/Right" : "Forward/Back";

        Serial.printf("[TILT] ✓ Motion detected! Angle: %.2f, Direction: %s (threshold: %.2f)\n",
                      angle, direction, tiltThreshold);

        triggerTiltResponse(angle, trackLeftRight);
    }
}

/**
 * Trigger tilt response based on angle and direction
 */
void KMMXController::triggerTiltResponse(float angle, bool isLeftRight) {
    if (isLeftRight) {
        // Left/Right tilt - curious/confused
        eyeState.setState(EyeStateEnum::DOUBTED);
        mouthState.setState(MouthStateEnum::EH);
        statusLED.setColor(Color::MAGENTA);  // Using MAGENTA instead of PURPLE

        if (enableMotionDebug) {
            Serial.printf("[TILT] Left/Right tilt response! Angle: %.2f\n", angle);
        }
    } else {
        // Forward/Back tilt
        if (angle > 0) {
            // Forward tilt (looking down) - unimpressed
            eyeState.setState(EyeStateEnum::UNIMPRESSED);
            mouthState.setState(MouthStateEnum::IDLE);
            statusLED.setColor(Color::BLUE);

            if (enableMotionDebug) {
                Serial.printf("[TILT] Forward tilt response! Angle: %.2f\n", angle);
            }
        } else {
            // Back tilt (looking up) - curious/surprised
            eyeState.setState(EyeStateEnum::ROUNDED);
            mouthState.setState(MouthStateEnum::IDLE);
            statusLED.setColor(Color::CYAN);

            if (enableMotionDebug) {
                Serial.printf("[TILT] Back tilt response! Angle: %.2f\n", angle);
            }
        }
    }

    // Note: Tilt does NOT reset idle timer (allows natural sleep if held still while tilted)
}

// Bounce Detection - REMOVED
// Spin Detection - REMOVED

/**
 * Petting Detection - Detects gentle rocking motion for contentment
 * Uses low-frequency oscillation detection in acceleration magnitude
 */
void KMMXController::detectPetting(const SensorData& current) {
    unsigned long currentTime = millis();

    // Check if we're in cooldown after tilt returns to neutral
    // Cooldown activates when ANY tilt (forward/back or left/right) returns to neutral
    if (enablePettingCooldownAfterTilt) {
        unsigned long timeSinceNeutral = currentTime - tiltDetector.lastNeutralReturnTime;
        if (timeSinceNeutral < pettingCooldownAfterTilt) {
            // Still in cooldown period after returning to neutral - skip petting detection
            if (enableMotionDebug && pettingDetector.isPetting) {
                Serial.printf("[PETTING] Cooldown active - %lu ms since neutral return (need %u ms)\n",
                              timeSinceNeutral, pettingCooldownAfterTilt);
            }
            return;
        }
    }

    // Add current magnitude to oscillation history
    pettingDetector.oscillationHistory[pettingDetector.historyIndex] = current.accelMagnitude;
    pettingDetector.historyIndex = (pettingDetector.historyIndex + 1) % 20;

    // Calculate average magnitude
    float avgMagnitude = 0.0f;
    for (uint8_t i = 0; i < 20; i++) {
        avgMagnitude += pettingDetector.oscillationHistory[i];
    }
    avgMagnitude /= 20;

    // Calculate oscillation amplitude (deviation from average)
    float oscillationAmplitude = fabsf(current.accelMagnitude - avgMagnitude);

    // Check if oscillation is in the gentle range
    bool isGentleOscillation = (oscillationAmplitude >= pettingMinMagnitude &&
                                 oscillationAmplitude <= pettingMaxMagnitude);

    if (isGentleOscillation) {
        if (!pettingDetector.isPetting) {
            // Start of petting
            pettingDetector.isPetting = true;
            pettingDetector.pettingStartTime = currentTime;
            pettingDetector.previousEyeState = eyeState.getState();
            pettingDetector.previousMouthState = mouthState.getState();

            if (enableMotionDebug) {
                Serial.printf("[PETTING] ✓ Motion detected! Amplitude: %.2f (range: %.2f-%.2f) | Saved previous state: Eye=%d, Mouth=%d\n",
                              oscillationAmplitude, pettingMinMagnitude, pettingMaxMagnitude,
                              pettingDetector.previousEyeState, pettingDetector.previousMouthState);
            }
        }

        pettingDetector.lastPettingTime = currentTime;

        // Check duration for petting trigger
        unsigned long pettingDuration = currentTime - pettingDetector.pettingStartTime;

        if (pettingDuration >= pettingMinDuration) {
            // Petting detected - show SMILE eyes
            // Continuously maintain SMILE response while petting
            if (eyeState.getState() != EyeStateEnum::SMILE) {
                triggerPettingResponse(false);
                if (enableMotionDebug) {
                    Serial.printf("[PETTING] Setting SMILE state (current: %d)\n", eyeState.getState());
                }
            }
        }
    } else {
        // Amplitude out of range
        if (pettingDetector.isPetting) {
            // Check if we should end immediately or wait for timeout
            if (currentTime - pettingDetector.lastPettingTime > 2000) {
                // End petting after timeout (stopped rocking)
                pettingDetector.isPetting = false;

                // Return to previous state (like other motion detectors)
                eyeState.setState(pettingDetector.previousEyeState);
                mouthState.setState(pettingDetector.previousMouthState);

                if (enableMotionDebug) {
                    Serial.printf("[PETTING] Ended - returning to previous state (Eye: %d, Mouth: %d)\n",
                                  pettingDetector.previousEyeState, pettingDetector.previousMouthState);
                }
            } else if (oscillationAmplitude > pettingMaxMagnitude) {
                // Amplitude exceeded maximum - end petting immediately
                pettingDetector.isPetting = false;

                // Return to previous state
                eyeState.setState(pettingDetector.previousEyeState);
                mouthState.setState(pettingDetector.previousMouthState);

                if (enableMotionDebug) {
                    Serial.printf("[PETTING] Ended - amplitude too high (%.2f > %.2f)\n",
                                  oscillationAmplitude, pettingMaxMagnitude);
                }
            }
        }
    }
}

/**
 * Trigger petting response - simplified to single SMILE state
 */
void KMMXController::triggerPettingResponse(bool sustained) {
    // Petting response - happy SMILE
    eyeState.setState(EyeStateEnum::SMILE);
    mouthState.setState(MouthStateEnum::IDLE);
    cheekPanel.setBrightness(200);
    statusLED.setColor(Color::PINK);

    if (enableMotionDebug) {
        Serial.println("[PETTING] Petting response! (SMILE)");
    }

    // Note: Petting does NOT reset idle timer (allows natural sleep transition)
}
