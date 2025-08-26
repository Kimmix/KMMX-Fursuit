#include "EyeState.h"
#include "Utils/Utils.h"
#include <Arduino.h>
#include "Bitmaps/Bitmaps.h"

EyeState::EyeState(Hub75DMA* display) : display(display), startSleepTime(millis()) {
    // Initialization if needed, but animation arrays are already initialized in the header file.
}

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

void EyeState::getListEvent(const sensors_event_t& eventData) {
    event = eventData;
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
    float zAcc = event.acceleration.z;
    const float leftThreshold = 3.00, rightThreshold = -3.00,
                leftMaxThreshold = 6.00, rightMaxThreshold = -6.00;
    if (zAcc > leftThreshold) {
        int level = mapFloat(zAcc, leftThreshold, leftMaxThreshold, 0, 19);
        display->drawEye(eyeUpAnimation[level], eyeDownAnimation[level]);
    } else if (zAcc < rightThreshold) {
        int level = mapFloat(zAcc, rightThreshold, rightMaxThreshold, 0, 19);
        display->drawEye(eyeDownAnimation[level], eyeUpAnimation[level]);
    } else {
        display->drawEye(eyeFrame);
    }
}

void EyeState::idleFace() {
    movingEye();
    if (millis() >= nextBlink) {
        // Natural blink patterns with simplified random selection
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

        currentState = EyeStateEnum::BLINK;
    }
}

void EyeState::blink() {
    if (millis() >= blinkInterval) {
        // Get base timing from lookup table
        uint8_t baseDelay = blinkTimings[blinkType][blinkDirection ? 0 : 1];

        // Add frame position variance (faster in middle of animation)
        int frameVariance = blinkDirection ?
            (currentBlinkFrameIndex < 7 ? -1 : 1) :
            ((blinkAnimationLength - currentBlinkFrameIndex) < 7 ? 2 : -1);

        // Apply variance and add randomness (±1ms)
        int frameDelay = baseDelay + frameVariance + ((esp_random() % 3) - 1);
        frameDelay = max(frameDelay, 2); // Minimum 2ms

        blinkInterval = millis() + frameDelay;

        // Update animation frame
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
                // Reset for next blink
                blinkDirection = true;
                currentBlinkFrameIndex = 0;
                changeDefaultFace();
                currentState = EyeStateEnum::IDLE;
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
    googlyEye.renderEye(event.acceleration.z, event.acceleration.x);
    display->drawEyePupil(eyePupil, googlyEye.x, googlyEye.y);
}
