#include "EyeState.h"
#include "Utils/Utils.h"
#include <Arduino.h>
#include "Bitmaps/Bitmaps.h"

EyeState::EyeState(Hub75DMA* display) : display(display), startSleepTime(millis()) {
    // Initialize smile transition animation (full animation, plays once)
    TimeBasedAnimation::init(smileAnim, smileAnimation, smileLength, TimeBasedAnimation::CONFIG_TRANSITION);

    // Initialize smile loop animation (last 5 frames only, ping-pong loop)
    // Uses pointer arithmetic to reference the last 5 frames without duplicating data
    TimeBasedAnimation::init(smileLoopAnim, &smileAnimation[smileLength - smileLoopFrames], smileLoopFrames, TimeBasedAnimation::CONFIG_SMILE_LOOP);

    // Initialize blink animation (24 frames, quick ping-pong)
    TimeBasedAnimation::init(blinkAnim, blinkAnimation, blinkAnimationLength, TimeBasedAnimation::CONFIG_BLINK);

    // Initialize boop transition animation (full animation, plays once)
    TimeBasedAnimation::init(boopAnim, boopAnimation, boopLength, TimeBasedAnimation::CONFIG_TRANSITION);

    // Initialize boop loop animation (last 10 frames only, ping-pong loop)
    // Uses pointer arithmetic to reference the last 10 frames without duplicating data
    TimeBasedAnimation::init(boopLoopAnim, &boopAnimation[boopLength - boopLoopFrames], boopLoopFrames, TimeBasedAnimation::CONFIG_QUICK_LOOP);

    // Initialize O-face transition animation (full animation, plays once)
    TimeBasedAnimation::init(oFaceAnim, oFaceAnimation, oFaceLength, TimeBasedAnimation::CONFIG_TRANSITION);

    // Initialize O-face loop animation (last 10 frames only, ping-pong loop)
    // Uses pointer arithmetic to reference the last 10 frames without duplicating data
    TimeBasedAnimation::init(oFaceLoopAnim, &oFaceAnimation[oFaceLength - oFaceLoopFrames], oFaceLoopFrames, TimeBasedAnimation::CONFIG_QUICK_LOOP);

    // Initialize angry transition animation (full animation, plays once)
    TimeBasedAnimation::init(angryAnim, eyeAngryAnimation, angryLength, TimeBasedAnimation::CONFIG_TRANSITION);

    // Initialize angry loop animation (last 10 frames only, ping-pong loop)
    // Uses pointer arithmetic to reference the last 10 frames without duplicating data
    TimeBasedAnimation::init(angryLoopAnim, &eyeAngryAnimation[angryLength - angryLoopFrames], angryLoopFrames, TimeBasedAnimation::CONFIG_SMILE_LOOP);

    // Initialize idle timers
    nextIdleAction = millis() + 1000;  // First idle action after 1s
    nextBlink = millis() + 3000;       // First blink after 3s
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
    if (newState == EyeStateEnum::SMILE) {
        // Reset smile animation to start from beginning with transition timing
        TimeBasedAnimation::setConfig(smileAnim, TimeBasedAnimation::CONFIG_SMILE_TRANSITION);
        TimeBasedAnimation::reset(smileAnim);
    }
    if (newState == EyeStateEnum::BLINK) {
        // Reset blink animation
        TimeBasedAnimation::reset(blinkAnim);
    }
    if (newState == EyeStateEnum::BOOP) {
        // Reset boop animation
        TimeBasedAnimation::reset(boopAnim);
    }
    if (newState == EyeStateEnum::OEYE) {
        // Reset O-face animation
        TimeBasedAnimation::reset(oFaceAnim);
    }
    if (newState == EyeStateEnum::ANGRY) {
        // Reset angry animation
        TimeBasedAnimation::reset(angryAnim);
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

void EyeState::movingEye() {
    // DISABLED: Accelerometer-based eye movement
    // Uncomment the code below to enable tilt-based eye animations

    /*
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
    */

    // Always render current idle frame (with micro-movements)
    display->drawEye(idleLookFrames[currentIdleFrame]);
}

void EyeState::idleFace() {
    movingEye();  // Renders the current idle frame
    updateIdleMicroMovements();
    checkAndTriggerBlink();
}

void EyeState::updateIdleMicroMovements() {
    if (millis() < nextIdleAction) return;

    uint32_t randomAction = esp_random() % 100;

    if (randomAction < 30) {
        // 30% - Quick eye dart (subtle look around)
        currentIdleFrame = (esp_random() % 5) + 1;
    } else if (randomAction < 50) {
        // 20% - Return to center
        currentIdleFrame = 0;
    }
    // 50% - Stay in current position (no change needed)

    // Schedule next micro-movement (800ms - 2000ms)
    nextIdleAction = millis() + 800 + (esp_random() % 1200);
}

void EyeState::checkAndTriggerBlink() {
    if (millis() < nextBlink) return;

    // Schedule next blink with varied interval
    nextBlink = millis() + selectBlinkInterval();

    // Decide if double blink (10% chance)
    shouldDoubleBlink = (esp_random() % 100) < 10;
    blinkCount = 0;

    // Select blink speed and initialize animation
    const TimeBasedAnimConfig* blinkConfig = selectBlinkSpeed(shouldDoubleBlink);
    TimeBasedAnimation::init(blinkAnim, blinkAnimation, blinkAnimationLength, *blinkConfig);

    currentState = EyeStateEnum::BLINK;
}

unsigned long EyeState::selectBlinkInterval() {
    uint32_t variance = esp_random() % 100;

    if (variance < 10) {
        // 10% - Quick interval (1-3s) - alert/active
        return 1000 + (esp_random() % 2000);
    } else if (variance < 80) {
        // 70% - Normal interval (5-5.7s)
        return 5000 + (esp_random() % 2500);
    } else {
        // 20% - Long interval (10-20s) - relaxed/zoned out
        return 10000 + (esp_random() % 10000);
    }
}

const TimeBasedAnimConfig* EyeState::selectBlinkSpeed(bool isDoubleBlink) {
    uint32_t variance = esp_random() % 100;

    if (variance < 15) {
        // 15% - Very fast (100ms) - surprised/alert
        return isDoubleBlink ? &TimeBasedAnimation::CONFIG_BLINK_VERY_FAST_DBL : &TimeBasedAnimation::CONFIG_BLINK_VERY_FAST;
    } else if (variance < 40) {
        // 25% - Fast (120ms) - active
        return isDoubleBlink ? &TimeBasedAnimation::CONFIG_BLINK_FAST_DBL : &TimeBasedAnimation::CONFIG_BLINK_FAST;
    } else if (variance < 75) {
        // 35% - Normal (150ms) - standard
        return isDoubleBlink ? &TimeBasedAnimation::CONFIG_BLINK_NORMAL_DBL : &TimeBasedAnimation::CONFIG_BLINK_NORMAL;
    } else if (variance < 90) {
        // 15% - Slow (200ms) - relaxed
        return isDoubleBlink ? &TimeBasedAnimation::CONFIG_BLINK_SLOW_DBL : &TimeBasedAnimation::CONFIG_BLINK_SLOW;
    } else {
        // 10% - Very slow (250ms) - tired/sleepy
        return isDoubleBlink ? &TimeBasedAnimation::CONFIG_BLINK_VERY_SLOW_DBL : &TimeBasedAnimation::CONFIG_BLINK_VERY_SLOW;
    }
}

void EyeState::blink() {
    const uint8_t* currentFrame = TimeBasedAnimation::update(blinkAnim);
    display->drawEye(currentFrame);

    // Check if blink animation is complete
    if (TimeBasedAnimation::isComplete(blinkAnim)) {
        blinkCount++;

        // Check if we should do a second blink
        if (shouldDoubleBlink && blinkCount == 1) {
            // Varied pause between double blinks for more personality
            uint32_t pauseVariance = esp_random() % 100;
            unsigned long pauseDuration;

            if (pauseVariance < 30) {
                // 30% - Very quick double blink (50-100ms) - rapid/surprised
                pauseDuration = 50 + (esp_random() % 50);
            } else if (pauseVariance < 70) {
                // 40% - Normal double blink (100-150ms) - standard
                pauseDuration = 100 + (esp_random() % 50);
            } else {
                // 30% - Slow double blink (150-250ms) - thoughtful/deliberate
                pauseDuration = 150 + (esp_random() % 100);
            }

            delay(pauseDuration);
            TimeBasedAnimation::reset(blinkAnim);
        } else {
            // Reset for next blink
            blinkCount = 0;
            shouldDoubleBlink = false;
            currentState = EyeStateEnum::IDLE;
        }
    }
}

void EyeState::arrowFace() {
    const uint8_t* currentFrame;

    if (isTransitioning) {
        // During transition: use full animation (frames 0-47)
        currentFrame = TimeBasedAnimation::update(boopAnim);

        // Check if transition animation is complete
        if (TimeBasedAnimation::isComplete(boopAnim)) {
            isTransitioning = false;
            // Reset the loop animation to start fresh
            TimeBasedAnimation::reset(boopLoopAnim);
        }
    } else {
        // After transition: loop only the last 10 frames (frames 38-47)
        currentFrame = TimeBasedAnimation::update(boopLoopAnim);
    }

    display->drawEye(currentFrame);
}

void EyeState::oFace() {
    const uint8_t* currentFrame;

    if (isTransitioning) {
        // During transition: use full animation (frames 0-47)
        currentFrame = TimeBasedAnimation::update(oFaceAnim);

        // Check if transition animation is complete
        if (TimeBasedAnimation::isComplete(oFaceAnim)) {
            isTransitioning = false;
            // Reset the loop animation to start fresh
            TimeBasedAnimation::reset(oFaceLoopAnim);
        }
    } else {
        // After transition: loop only the last 10 frames (frames 38-47)
        currentFrame = TimeBasedAnimation::update(oFaceLoopAnim);
    }

    display->drawEye(currentFrame);
}

void EyeState::smileFace() {
    const uint8_t* currentFrame;

    if (isTransitioning) {
        // During transition: use full animation (frames 0-19)
        currentFrame = TimeBasedAnimation::update(smileAnim);

        // Check if transition animation is complete
        if (TimeBasedAnimation::isComplete(smileAnim)) {
            isTransitioning = false;
            // Reset the loop animation to start fresh
            TimeBasedAnimation::reset(smileLoopAnim);
        }
    } else {
        // After transition: loop only the last 5 frames (frames 15-19)
        currentFrame = TimeBasedAnimation::update(smileLoopAnim);
    }

    display->drawEye(currentFrame);
}

void EyeState::angryFace() {
    const uint8_t* currentFrame;

    if (isTransitioning) {
        // During transition: use full animation (frames 0-47)
        currentFrame = TimeBasedAnimation::update(angryAnim);

        // Check if transition animation is complete
        if (TimeBasedAnimation::isComplete(angryAnim)) {
            isTransitioning = false;
            // Reset the loop animation to start fresh
            TimeBasedAnimation::reset(angryLoopAnim);
        }
    } else {
        // After transition: loop only the last 10 frames (frames 38-47)
        currentFrame = TimeBasedAnimation::update(angryLoopAnim);
    }

    display->drawEye(currentFrame);
}

int EyeState::calculateSleepIndex(int currentIndex) {
    unsigned long elapsedTime = millis() - startSleepTime;
    // Calculate minIndex based on elapsed time (updated for 48 frames)
    // Increased from 5000 to 10000 to slow down the progression
    int minIndex = min(pow(elapsedTime / 10000.0, 2), (double)(sleepLength - 1));
    // Calculate decrease probability based on elapsed time
    // Increased from 20000 to 40000 to slow down the closing probability
    float decreaseProbability = min(1.0f, elapsedTime / 40000.0f);
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

    // Check interval: how often we evaluate whether to change frames (responsive)
    const unsigned long CHECK_INTERVAL = 100;  // Check every 100ms for smooth response

    // Frame change interval: minimum time between actual frame changes (controls visual speed)
    const unsigned long FRAME_CHANGE_INTERVAL = 400;  // Only allow frame changes every 400ms

    if (currentMillis >= nextSleep) {
        // Time to check if we should change frames
        if (currentMillis - lastSleepFrameChange >= FRAME_CHANGE_INTERVAL) {
            // Enough time has passed since last frame change, allow a new one
            if (sleepIndex < sleepLength - 1) {
                sleepIndex = calculateSleepIndex(sleepIndex);
                lastSleepFrameChange = currentMillis;
            }
        }
        // Schedule next check (independent of frame changes)
        nextSleep = currentMillis + CHECK_INTERVAL;
    }
    display->drawEye(eyeSleepAnimation[sleepIndex]);
}

void EyeState::resetSleepFace() {
    sleepIndex = 0;
    startSleepTime = millis();
    lastSleepFrameChange = millis();  // Initialize frame change timer
}

void EyeState::detransition() {
}

void EyeState::renderGooglyEye() {
    display->drawEye(eyeGoogly);
    googlyEye.renderEye(sensorData.accelZ, sensorData.accelX);
    display->drawEyePupil(eyePupil, googlyEye.x, googlyEye.y);
}
