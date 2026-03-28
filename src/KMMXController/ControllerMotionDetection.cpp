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

    // Get current sensor data from active buffer
    const SensorData& current = controller->sensorBuffer[controller->activeBuffer];

    // Upside-down detection runs FIRST - even in high-priority states
    // This allows detecting when to exit crying state
    if (enableUpsideDownDetection) {
        controller->detectUpsideDown(current);
        if (controller->upsideDownDetector.isUpsideDown) return;
    }

    // Skip other motion detection if in high-priority states
    EyeStateEnum currentEyeState = controller->eyeState.getState();

    // Don't interfere with boop, manual states, or transitions
    if (currentEyeState == EyeStateEnum::BOOP ||
        currentEyeState == EyeStateEnum::ANGRY ||
        currentEyeState == EyeStateEnum::CRY) {
        return;  // Skip remaining motion detection
    }

    // Check remaining features in priority order (first match wins)
    if (enableTiltDetection) {
        controller->detectTilt(current);
        if (controller->tiltDetector.isTilted) return;
    }

    if (enablePettingDetection) {
        controller->detectPetting(current);
    }
}

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
}

/**
 * Upside Down Detection - Detects when the character is held upside down
 * Uses Y-axis accelerometer to detect upside-down orientation
 */
void KMMXController::detectUpsideDown(const SensorData& current) {
    unsigned long currentTime = millis();
    float yAxis = current.accelY;
    bool isFlipped = yAxis > -upsideDownThreshold;

    // Debounce - prevent rapid state changes
    if (currentTime - upsideDownDetector.lastStateChangeTime < upsideDownDebounceTime) {
        return;
    }

    // ========== STATE 1: Currently Upside Down ==========
    if (upsideDownDetector.isUpsideDown) {
        // Check if returning to normal orientation
        if (!isFlipped) {
            // Return to normal
            upsideDownDetector.isUpsideDown = false;
            upsideDownDetector.upsideDownStartTime = 0;
            upsideDownDetector.lastStateChangeTime = currentTime;

            eyeState.setState(upsideDownDetector.previousEyeState);
            mouthState.setState(upsideDownDetector.previousMouthState);

            if (enableMotionDebug) {
                Serial.println("[UPSIDE DOWN] Returned to normal orientation");
            }
        }
        // If still upside down, maintain crying state (no action needed)
        return;
    }

    // ========== STATE 2: Not Currently Upside Down ==========
    if (!isFlipped) {
        // Normal orientation - reset any tracking
        if (upsideDownDetector.upsideDownStartTime != 0) {
            upsideDownDetector.upsideDownStartTime = 0;  // Reset tracking silently
        }
        return;
    }

    // Upside down detected - start or continue tracking
    if (upsideDownDetector.upsideDownStartTime == 0) {
        // First time detecting upside down - start tracking
        upsideDownDetector.upsideDownStartTime = currentTime;
        if (enableMotionDebug) {
            Serial.printf("[UPSIDE DOWN] Starting tracking... Y-axis: %.2f (threshold: %.2f)\n",
                          yAxis, upsideDownThreshold);
        }
        return;
    }

    // Already tracking - check if sustained long enough
    if (currentTime - upsideDownDetector.upsideDownStartTime >= upsideDownSustainTime) {
        // Trigger upside down response (crying)
        upsideDownDetector.isUpsideDown = true;
        upsideDownDetector.previousEyeState = eyeState.getState();
        upsideDownDetector.previousMouthState = mouthState.getState();
        upsideDownDetector.lastStateChangeTime = currentTime;

        Serial.printf("[UPSIDE DOWN] ✓ Upside down detected! Y-axis: %.2f (threshold: %.2f)\n",
                      yAxis, upsideDownThreshold);

        triggerUpsideDownResponse();
    }
}

/**
 * Trigger upside down response - crying
 */
void KMMXController::triggerUpsideDownResponse() {
    eyeState.setState(EyeStateEnum::CRY);
    mouthState.setState(MouthStateEnum::WAH);
    statusLED.setColor(Color::BLUE);

    if (enableMotionDebug) {
        Serial.println("[UPSIDE DOWN] Crying response triggered!");
    }
}

/**
 * Petting Detection - Dynamic happiness accumulation system
 * Each pat adds happiness which decays over time. More natural and responsive than fixed time windows.
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

    // ========== Initialize last update time on first run ==========
    if (pettingDetector.lastUpdateTime == 0) {
        pettingDetector.lastUpdateTime = currentTime;
    }

    // ========== Calculate happiness decay ==========
    // Happiness naturally decays over time (like a leaky bucket)
    float deltaTime = (currentTime - pettingDetector.lastUpdateTime) / 1000.0f;  // Convert to seconds
    pettingDetector.lastUpdateTime = currentTime;

    if (deltaTime > 0 && deltaTime < 1.0f) {  // Sanity check (ignore huge time jumps)
        float decay = pettingHappinessDecayRate * deltaTime;
        pettingDetector.happiness -= decay;
        if (pettingDetector.happiness < 0) {
            pettingDetector.happiness = 0;
        }
    }

    // ========== Spike Detection ==========
    // Calculate magnitude change (derivative)
    float magnitudeChange = fabsf(current.accelMagnitude - pettingDetector.lastMagnitude);
    pettingDetector.lastMagnitude = current.accelMagnitude;

    // Detect spike: sudden increase in acceleration magnitude
    bool spikeDetected = (magnitudeChange >= pettingSpikeThreshold);

    // Check spike cooldown to prevent double-counting
    if (spikeDetected && (currentTime - pettingDetector.lastSpikeTime >= pettingSpikeCooldown)) {
        // Valid spike detected! Add happiness
        pettingDetector.lastSpikeTime = currentTime;
        pettingDetector.happiness += pettingHappinessPerPat;

        // Cap happiness at 100
        if (pettingDetector.happiness > 100.0f) {
            pettingDetector.happiness = 100.0f;
        }

        if (enableMotionDebug) {
            Serial.printf("[PETTING] Pat detected! Change: %.2f m/s² | Happiness: %.1f/%.1f\n",
                          magnitudeChange, pettingDetector.happiness, pettingHappinessTrigger);
        }
    }

    // ========== Check happiness thresholds ==========
    if (!pettingDetector.isPetting) {
        // Not currently showing response - check if happiness reached trigger threshold
        if (pettingDetector.happiness >= pettingHappinessTrigger) {
            // Trigger petting response!
            pettingDetector.isPetting = true;
            pettingDetector.previousEyeState = eyeState.getState();
            pettingDetector.previousMouthState = mouthState.getState();

            if (enableMotionDebug) {
                Serial.printf("[PETTING] ✓ Petting triggered! Happiness: %.1f | Saved previous state: Eye=%d, Mouth=%d\n",
                              pettingDetector.happiness,
                              pettingDetector.previousEyeState, pettingDetector.previousMouthState);
            }

            triggerPettingResponse(false);
        }
    } else {
        // Currently showing response - check if happiness dropped below end threshold
        if (pettingDetector.happiness < pettingHappinessEndThreshold) {
            // End petting response (happiness decayed naturally)
            pettingDetector.isPetting = false;
            pettingDetector.happiness = 0;  // Reset happiness

            // Return to previous state
            eyeState.setState(pettingDetector.previousEyeState);
            mouthState.setState(pettingDetector.previousMouthState);

            if (enableMotionDebug) {
                Serial.printf("[PETTING] Response ended naturally (happiness decayed to %.1f) - returning to previous state (Eye: %d, Mouth: %d)\n",
                              pettingDetector.happiness,
                              pettingDetector.previousEyeState, pettingDetector.previousMouthState);
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
}
