#include "EyeState.h"
#include "Utils/Utils.h"
#include <Arduino.h>
#include "Bitmaps/Bitmaps.h"

EyeState::EyeState(Hub75DMA* display) : display(display), startSleepTime(millis()) {}

void EyeState::update() {
    switch (currentState) {
        case EyeStateEnum::IDLE:
            idleFace();
            break;
        case EyeStateEnum::BLINK:
            blink();
            break;
        case EyeStateEnum::BOOP:
            arrowFace();
            if (millis() - resetBoop >= 1500) {
                currentState = prevState;
            }
            break;
        case EyeStateEnum::GOOGLY:
            renderGooglyEye();
            break;
        case EyeStateEnum::OEYE:
            oFace();
            break;
        case EyeStateEnum::HEART:
            display->drawEye(eyeHeart);
            break;
        case EyeStateEnum::SMILE:
            smileFace();
            break;
        case EyeStateEnum::ANGRY:
            angryFace();
            if (millis() - resetBoop >= 2000) {
                currentState = prevState;
            }
            break;
        case EyeStateEnum::SLEEP:
            sleepFace();
            break;
        case EyeStateEnum::DETRANSITION:
            detransition();
            break;
        default:
            break;
    }
}

void EyeState::setState(EyeStateEnum newState) {
    if (newState == EyeStateEnum::BOOP || newState == EyeStateEnum::ANGRY) {
        resetBoop = millis();
    }
    if (newState == EyeStateEnum::SLEEP) {
        resetSleepFace();
    }
    if (currentState != newState) {
        isTransitioning = true;
    }
    currentState = newState;
    savePrevState(currentState);
}

void EyeState::savePrevState(EyeStateEnum newState) {
    if (newState == EyeStateEnum::BOOP || newState == EyeStateEnum::ANGRY || newState == EyeStateEnum::SLEEP) {
        return;
    }
    prevState = newState;
}

void EyeState::playPrevState() {
    setState(prevState);
    resetSleepFace();
}

EyeStateEnum EyeState::getState() const {
    return currentState;
}

void EyeState::setSensorData(const SensorData& data) {
    sensorData = data;
}

void EyeState::changeDefaultFace() {
    if ((esp_random() % 10) <= 3) {
        defaultAnimationIndex = (esp_random() % 2) + 1;
    } else {
        defaultAnimationIndex = 0;
    }
    eyeFrame = defaultAnimation[defaultAnimationIndex];
}

void EyeState::movingEye() {
    float zAcc = sensorData.accelZ;
    const float leftThreshold = 3.00, rightThreshold = -3.00,
                leftMaxThreshold = 6.00, rightMaxThreshold = -6.00;
    static float smoothedLeftAcc = 0.0f;
    static float smoothedRightAcc = 0.0f;
    // Check for left movement (positive direction)
    int leftLevel = smoothAccelerometerMovement(zAcc, smoothedLeftAcc, leftThreshold, leftMaxThreshold, 0.3f, 0.5f, 19, false);
    if (leftLevel >= 0) {
        display->drawEye(eyeUpAnimation[leftLevel], eyeDownAnimation[leftLevel]);
        return;
    }
    // Check for right movement (negative direction)
    int rightLevel = smoothAccelerometerMovement(zAcc, smoothedRightAcc, rightThreshold, rightMaxThreshold, 0.3f, 0.5f, 19, true);
    if (rightLevel >= 0) {
        display->drawEye(eyeDownAnimation[rightLevel], eyeUpAnimation[rightLevel]);
        return;
    }
    // Default eye frame when no significant movement
    display->drawEye(eyeFrame);
}

void EyeState::idleFace() {
    movingEye();
    if (millis() >= nextBlink) {
        uint8_t randValue = esp_random() % 100;
        if (randValue < 70) {
            // 70% normal interval (2-6s) + normal blink
            nextBlink = millis() + 2000 + (esp_random() % 4000);
            blinkType = 0;
        } else if (randValue < 90) {
            // 20% quick interval (0.5-1.5s) + quick blink
            nextBlink = millis() + 500 + (esp_random() % 1000);
            blinkType = 1;
        } else {
            // 10% long pause (8-15s) + slow blink
            nextBlink = millis() + 8000 + (esp_random() % 7000);
            blinkType = 2;
        }

        // 25% chance for double blink
        shouldDoubleBlink = (esp_random() % 100) < 25;
        blinkCount = 0;

        currentState = EyeStateEnum::BLINK;
    }
}

void EyeState::blink() {
    if (millis() >= blinkInterval) {
        // Get timing from lookup table
        uint8_t frameDelay = blinkTimings[blinkType][blinkDirection ? 0 : 1];
        blinkInterval = millis() + frameDelay;
        if (blinkDirection) {
            // Closing eye
            if (++currentBlinkFrameIndex >= blinkAnimationLength) {
                blinkDirection = false;
                currentBlinkFrameIndex--;
                blinkInterval = millis() + 20 + (esp_random() % 20); // Pause when closed
            }
        } else {
            // Opening eye
            if (--currentBlinkFrameIndex <= 0) {
                blinkCount++;

                // Check if we should do a second blink
                if (shouldDoubleBlink && blinkCount == 1) {
                    // Start second blink after short pause (100-200ms)
                    blinkDirection = true;
                    currentBlinkFrameIndex = 0;
                    blinkInterval = millis() + 100 + (esp_random() % 100);
                } else {
                    // Reset for next blink
                    blinkDirection = true;
                    currentBlinkFrameIndex = 0;
                    blinkCount = 0;
                    shouldDoubleBlink = false;
                    changeDefaultFace();
                    currentState = EyeStateEnum::IDLE;
                }
            }
        }
    }
    display->drawEye(blinkAnimation[currentBlinkFrameIndex]);
}

void EyeState::arrowFace() {
    if (millis() > nextBoop) {
        nextBoop = millis() + 300;
        boopAnimationFrame ^= 1;
    }
    display->drawEye(boopAnimation[boopAnimationFrame]);
}

void EyeState::oFace() {
    if (millis() >= nextBoop) {
        nextBoop = millis() + 200;
        currentOFaceIndex = esp_random() % 3;
    }
    display->drawEye(oFaceAnimation[currentOFaceIndex]);
}

void EyeState::smileFace() {
    if (isTransitioning) {
        display->drawEye(smileAnimation[smileIndex]);
        if (millis() >= nextSmile) {
            nextSmile = millis() + 14;
            smileIndex++;
            if (smileIndex == smileLength) {
                smileIndex = 0;
                isTransitioning = false;
            }
        }
    } else {
        display->drawEye(smileAnimation[smileLength - 1]);
    }
}

void EyeState::angryFace() {
    if (isTransitioning) {
        display->drawEye(eyeangryAnimation[angryIndex]);
        if (millis() >= nextAngry) {
            nextAngry = millis() + 5;
            angryIndex++;
            if (angryIndex == angryLength) {
                angryIndex = 0;
                isTransitioning = false;
            }
        }
    } else {
        display->drawEye(eyeangryAnimation[angryLength - 9]);
    }
}

int EyeState::calculateSleepIndex(int currentIndex) {
    unsigned long elapsedTime = millis() - startSleepTime;
    // Calculate minIndex based on elapsed time
    int minIndex = min(pow(elapsedTime / 5000.0, 2), 19.0);
    // Calculate decrease probability based on elapsed time
    float decreaseProbability = min(1.0f, elapsedTime / 20000.0f);
    // Randomly determine whether to increase or decrease the index
    if (esp_random() % 100 < (100 * decreaseProbability)) {
        // Chance to increase index
        return min(currentIndex + 1, sleepLength - 1);
    } else {
        // Chance to decrease index
        return max(currentIndex - 1, minIndex);
    }
}

void EyeState::sleepFace() {
    unsigned long currentMillis = millis();
    if (currentMillis >= nextSleep) {
        if (sleepIndex < sleepLength - 1) {
            sleepIndex = calculateSleepIndex(sleepIndex);
            nextSleep = currentMillis + 300;
        }
    }
    display->drawEye(eyesleepAnimation[sleepIndex]);
}

void EyeState::resetSleepFace() {
    sleepIndex = 0;
    startSleepTime = millis();
}

void EyeState::detransition() {
}

void EyeState::renderGooglyEye() {
    display->drawEye(eyeGoogly);
    googlyEye.renderEye(sensorData.accelZ, sensorData.accelX);
    display->drawEyePupil(eyePupil, googlyEye.x, googlyEye.y);
}
