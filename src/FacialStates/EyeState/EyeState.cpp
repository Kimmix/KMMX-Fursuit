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

    // Initialize boop animation (2 frames, quick loop)
    TimeBasedAnimation::init(boopAnim, boopAnimation, 2, TimeBasedAnimation::CONFIG_QUICK_LOOP);

    // Initialize O-face animation (3 frames, quick loop)
    TimeBasedAnimation::init(oFaceAnim, oFaceAnimation, 3, TimeBasedAnimation::CONFIG_QUICK_LOOP);

    // Initialize angry animation (20 frames, quick transition)
    TimeBasedAnimation::init(angryAnim, eyeAngryAnimation, angryLength, TimeBasedAnimation::CONFIG_TRANSITION);

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
        TimeBasedAnimConfig transitionConfig = {
            .durationMs = 500,                          // 500ms for transition
            .playMode = AnimationPlayMode::ONCE,        // Play once to end
            .pauseAtEndMs = 0,
            .pauseAtStartMs = 0,
            .useEasing = true
        };
        TimeBasedAnimation::setConfig(smileAnim, transitionConfig);
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

void EyeState::changeDefaultFace() {
    // 40% chance to look in a different direction
    if ((esp_random() % 10) <= 3) {
        defaultAnimationIndex = (esp_random() % 2) + 1;  // eyeUp20 or eyeLookSharp5
    } else {
        defaultAnimationIndex = 0;  // eyeDefault (center)
    }
    eyeFrame = defaultAnimation[defaultAnimationIndex];

    // Also update the current idle frame to match
    currentIdleFrame = 0;  // Reset to default
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
    // Use current idle frame (with micro-movements)
    display->drawEye(idleLookFrames[currentIdleFrame]);
}

void EyeState::idleFace() {
    movingEye();

    // Micro-movements and eye darts (every 800ms - 2s)
    if (millis() >= nextIdleAction) {
        uint32_t randomAction = esp_random() % 100;

        if (randomAction < 30) {
            // 30% - Quick eye dart (subtle look around)
            currentIdleFrame = (esp_random() % 5) + 1;  // Pick from eyeUp5, eyeUp10, eyeLookSharp5, eyeLookSharp10, eyeGiggle8
        } else if (randomAction < 50) {
            // 20% - Return to base position
            currentIdleFrame = 0;  // eyeDefault
        } else {
            // 50% - Stay in current position (no change)
        }

        // Set next idle action (800ms - 2000ms)
        nextIdleAction = millis() + 800 + (esp_random() % 1200);
    }

    // Blink timing with more variance
    if (millis() >= nextBlink) {
        // Varied blink intervals based on randomness
        uint32_t blinkVariance = esp_random() % 100;
        unsigned long blinkDelay;

        if (blinkVariance < 10) {
            // 10% - Quick blink (1-2s) - more alert/active
            blinkDelay = 1000 + (esp_random() % 1000);
        } else if (blinkVariance < 80) {
            // 70% - Normal blink (2.5-5s)
            blinkDelay = 2500 + (esp_random() % 2500);
        } else {
            // 20% - Long pause (5-8s) - relaxed/zoned out
            blinkDelay = 5000 + (esp_random() % 3000);
        }

        nextBlink = millis() + blinkDelay;

        // 30% chance for double blink (decide first to adjust pause timing)
        shouldDoubleBlink = (esp_random() % 100) < 30;
        blinkCount = 0;

        // Randomize blink SPEED for this blink (vary animation duration)
        uint32_t speedVariance = esp_random() % 100;
        TimeBasedAnimConfig customBlinkConfig;

        if (speedVariance < 15) {
            // 15% - Very fast blink (100ms) - surprised/alert
            customBlinkConfig = {.durationMs = 100, .playMode = AnimationPlayMode::ONCE, .pauseAtEndMs = (unsigned long)(shouldDoubleBlink ? 0 : 30), .pauseAtStartMs = 0, .useEasing = false};
        } else if (speedVariance < 40) {
            // 25% - Fast blink (120ms) - active
            customBlinkConfig = {.durationMs = 120, .playMode = AnimationPlayMode::ONCE, .pauseAtEndMs = (unsigned long)(shouldDoubleBlink ? 0 : 40), .pauseAtStartMs = 0, .useEasing = false};
        } else if (speedVariance < 75) {
            // 35% - Normal blink (150ms) - standard
            customBlinkConfig = {.durationMs = 150, .playMode = AnimationPlayMode::ONCE, .pauseAtEndMs = (unsigned long)(shouldDoubleBlink ? 0 : 50), .pauseAtStartMs = 0, .useEasing = false};
        } else if (speedVariance < 90) {
            // 15% - Slow blink (200ms) - relaxed
            customBlinkConfig = {.durationMs = 200, .playMode = AnimationPlayMode::ONCE, .pauseAtEndMs = (unsigned long)(shouldDoubleBlink ? 0 : 60), .pauseAtStartMs = 0, .useEasing = false};
        } else {
            // 10% - Very slow blink (250ms) - tired/sleepy
            customBlinkConfig = {.durationMs = 250, .playMode = AnimationPlayMode::ONCE, .pauseAtEndMs = (unsigned long)(shouldDoubleBlink ? 0 : 80), .pauseAtStartMs = 0, .useEasing = false};
        }

        // Re-initialize blink animation with new speed
        TimeBasedAnimation::init(blinkAnim, blinkAnimation, blinkAnimationLength, customBlinkConfig);

        currentState = EyeStateEnum::BLINK;
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
            changeDefaultFace();
            currentState = EyeStateEnum::IDLE;
        }
    }
}

void EyeState::arrowFace() {
    display->drawEye(TimeBasedAnimation::update(boopAnim));
}

void EyeState::oFace() {
    display->drawEye(TimeBasedAnimation::update(oFaceAnim));
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
    const uint8_t* currentFrame = TimeBasedAnimation::update(angryAnim);

    if (isTransitioning) {
        display->drawEye(currentFrame);

        // Check if transition animation is complete
        if (TimeBasedAnimation::isComplete(angryAnim)) {
            isTransitioning = false;
        }
    } else {
        // Hold at frame near the end (angryLength - 9)
        display->drawEye(eyeAngryAnimation[angryLength - 9]);
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
    display->drawEye(eyeSleepAnimation[sleepIndex]);
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
