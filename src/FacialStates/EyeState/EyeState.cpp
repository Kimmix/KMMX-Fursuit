#include "EyeState.h"
#include "Utils/Utils.h"
#include <Arduino.h>
#include "Bitmaps/Bitmaps.h"

EyeState::EyeState(Hub75DMA* display) : display(display), startSleepTime(millis()) {
    // Initialize blink animation (special case - no loop)
    TimeBasedAnimation::init(blinkAnim, blinkAnimation, blinkAnimationLength, TimeBasedAnimation::CONFIG_BLINK);

    // Initialize all animations with transition + loop pattern
    initAnimationData(boopData, boopAnimation, 48, 18, 2500, TimeBasedAnimation::CONFIG_BOUNCE_OVERSHOOT);  // Auto-reset after 2.5s
    initAnimationData(arrowData, boopAnimation, 48, 18, 0, TimeBasedAnimation::CONFIG_BOUNCE_OVERSHOOT);  // No auto-reset (duplicate of boop for BLE)
    initAnimationData(oFaceData, oFaceAnimation, 48, 10, 0, TimeBasedAnimation::CONFIG_SMOOTH_LOOP);      // No auto-reset
    initAnimationData(smileData, smileAnimation, 48, 10, 0, TimeBasedAnimation::CONFIG_SMILE_LOOP);     // No auto-reset, special ping-pong loop
    initAnimationData(angryData, eyeAngryAnimation, 48, 18, 0, TimeBasedAnimation::CONFIG_ANTICIPATION);  // No auto-reset
    initAnimationData(sadData, sadAnimation, 20, 8, 3000, TimeBasedAnimation::CONFIG_SMOOTH_LOOP);  // Auto-reset after 3s
    initAnimationData(cryData, cryAnimation, 48, 12, 0, TimeBasedAnimation::CONFIG_SMOOTH_LOOP);  // No auto-reset, for photo shoots
    initAnimationData(doubtedData, doubtedAnimation, 48, 10, 0, TimeBasedAnimation::CONFIG_SMOOTH_LOOP);       // No auto-reset, for photo shoots
    initAnimationData(roundedData, roundedAnimation, 48, 15, 0, TimeBasedAnimation::CONFIG_BOUNCE_OVERSHOOT);  // No auto-reset, for photo shoots
    initAnimationData(sharpData, sharpAnimation, 48, 18, 0, TimeBasedAnimation::CONFIG_ANTICIPATION);  // No auto-reset, focused state
    initAnimationData(giggleData, giggleAnimation, 48, 12, 0, TimeBasedAnimation::CONFIG_SMILE_LOOP);  // No auto-reset, playful state
    initAnimationData(unimpressedData, unimpressedAnimation, 48, 14, 0, TimeBasedAnimation::CONFIG_SMOOTH_LOOP);  // No auto-reset, for photo shoots

    // Initialize idle timers
    nextIdleAction = millis() + 1000;  // First idle action after 1s
    nextBlink = millis() + 3000;       // First blink after 3s
}

void EyeState::initAnimationData(AnimationData& data, const uint8_t** frames, uint8_t frameCount,
                                 uint8_t loopFrameCount, unsigned long autoResetDuration,
                                 const TimeBasedAnimConfig& loopConfig) {
    data.frames = frames;
    data.frameCount = frameCount;
    data.loopFrameCount = loopFrameCount;
    data.autoResetDuration = autoResetDuration;

    // Initialize transition animation (full animation, plays once)
    TimeBasedAnimation::init(data.transitionAnim, frames, frameCount, TimeBasedAnimation::CONFIG_TRANSITION);

    // Initialize loop animation (last N frames only)
    // Uses pointer arithmetic to reference the last N frames without duplicating data
    TimeBasedAnimation::init(data.loopAnim, &frames[frameCount - loopFrameCount], loopFrameCount, loopConfig);
}

void EyeState::resetAnimation(AnimationData& data) {
    TimeBasedAnimation::reset(data.transitionAnim);
    TimeBasedAnimation::reset(data.loopAnim);
}

AnimationData* EyeState::getAnimationData(EyeStateEnum state) {
    switch (state) {
        case EyeStateEnum::BOOP:        return &boopData;
        case EyeStateEnum::ARROW:       return &arrowData;
        case EyeStateEnum::OEYE:        return &oFaceData;
        case EyeStateEnum::SMILE:       return &smileData;
        case EyeStateEnum::ANGRY:       return &angryData;
        case EyeStateEnum::SAD:         return &sadData;
        case EyeStateEnum::CRY:         return &cryData;
        case EyeStateEnum::DOUBTED:     return &doubtedData;
        case EyeStateEnum::ROUNDED:     return &roundedData;
        case EyeStateEnum::SHARP:       return &sharpData;
        case EyeStateEnum::GIGGLE:      return &giggleData;
        case EyeStateEnum::UNIMPRESSED: return &unimpressedData;
        default: return nullptr;
    }
}

void EyeState::update() {
    // Check for auto-reset states
    AnimationData* animData = getAnimationData(currentState);
    if (animData && animData->autoResetDuration > 0) {
        if (millis() - stateStartTime >= animData->autoResetDuration) {
            currentState = prevState;
        }
    }

    switch (currentState) {
        case EyeStateEnum::IDLE:
            idleFace();
            break;
        case EyeStateEnum::BLINK:
            blink();
            break;
        case EyeStateEnum::BOOP:
            playAnimationWithLoop(boopData);
            break;
        case EyeStateEnum::ARROW:
            playAnimationWithLoop(arrowData);
            break;
        case EyeStateEnum::GOOGLY:
            renderGooglyEye();
            break;
        case EyeStateEnum::OEYE:
            playAnimationWithLoop(oFaceData);
            break;
        case EyeStateEnum::HEART:
            display->drawEye(eyeHeart);
            break;
        case EyeStateEnum::SMILE:
            playAnimationWithLoop(smileData);
            break;
        case EyeStateEnum::ANGRY:
            playAnimationWithLoop(angryData);
            break;
        case EyeStateEnum::SLEEP:
            sleepFace();
            break;
        case EyeStateEnum::SAD:
            playAnimationWithLoop(sadData);
            break;
        case EyeStateEnum::CRY:
            playAnimationWithLoop(cryData);
            break;
        case EyeStateEnum::DOUBTED:
            playAnimationWithLoop(doubtedData);
            break;
        case EyeStateEnum::ROUNDED:
            playAnimationWithLoop(roundedData);
            break;
        case EyeStateEnum::SHARP:
            playAnimationWithLoop(sharpData);
            break;
        case EyeStateEnum::GIGGLE:
            playAnimationWithLoop(giggleData);
            break;
        case EyeStateEnum::UNIMPRESSED:
            playAnimationWithLoop(unimpressedData);
            break;
        case EyeStateEnum::DETRANSITION:
            detransition();
            break;
        default:
            break;
    }
}

void EyeState::setState(EyeStateEnum newState) {
    // Mark as transitioning if state is changing
    if (currentState != newState) {
        isTransitioning = true;
    }

    // State-specific initialization
    switch (newState) {
        case EyeStateEnum::BLINK:
            TimeBasedAnimation::reset(blinkAnim);
            break;

        case EyeStateEnum::SLEEP:
            resetSleepFace();
            break;

        case EyeStateEnum::SMILE:
            // Smile uses special transition config
            TimeBasedAnimation::setConfig(smileData.transitionAnim, TimeBasedAnimation::CONFIG_SMILE_TRANSITION);
            resetAnimation(smileData);
            break;

        case EyeStateEnum::BOOP:
        case EyeStateEnum::ARROW:
        case EyeStateEnum::OEYE:
        case EyeStateEnum::ANGRY:
        case EyeStateEnum::SAD:
        case EyeStateEnum::CRY:
        case EyeStateEnum::DOUBTED:
        case EyeStateEnum::ROUNDED:
        case EyeStateEnum::SHARP:
        case EyeStateEnum::GIGGLE:
        case EyeStateEnum::UNIMPRESSED: {
            // Reset animation data for states with transition + loop
            AnimationData* animData = getAnimationData(newState);
            if (animData) {
                resetAnimation(*animData);
            }
            break;
        }

        default:
            break;
    }

    // Update state and timestamp
    currentState = newState;
    stateStartTime = millis();
    savePrevState(currentState);
}

void EyeState::savePrevState(EyeStateEnum newState) {
    // Don't save states that auto-reset as previous state
    if (newState == EyeStateEnum::BOOP ||
        newState == EyeStateEnum::SLEEP ||
        newState == EyeStateEnum::SAD) {
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
        currentIdleFrame = esp_random() % idleLookFramesLength;
    } else if (randomAction < 50) {
        // 20% - Return to center
        currentIdleFrame = 0;
    }
    // 50% - Stay in current position (no change needed)

    // Schedule next micro-movement (800ms - 4000ms)
    nextIdleAction = millis() + 800 + (esp_random() % 3200);
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

void EyeState::playAnimationWithLoop(AnimationData& animData) {
    const uint8_t* currentFrame;

    if (isTransitioning) {
        // During transition: use full animation
        currentFrame = TimeBasedAnimation::update(animData.transitionAnim);

        // Also update loop animation in the background so it's in sync when we switch
        TimeBasedAnimation::update(animData.loopAnim);

        // Check if transition animation is complete
        if (TimeBasedAnimation::isComplete(animData.transitionAnim)) {
            isTransitioning = false;
            // Loop animation is already running in sync, no reset needed
        }
    } else {
        // After transition: loop only the last N frames
        currentFrame = TimeBasedAnimation::update(animData.loopAnim);
    }

    display->drawEye(currentFrame);
}

void EyeState::sleepFace() {
    unsigned long currentMillis = millis();
    unsigned long elapsedTime = currentMillis - startSleepTime;

    // Time it takes to fully close eyes (in milliseconds)
    const unsigned long FULL_CLOSE_DURATION = 30000;  // 30 seconds to fully close

    // Calculate target frame based on elapsed time (linear progression)
    uint8_t targetIndex = min((elapsedTime * sleepLength) / FULL_CLOSE_DURATION, (unsigned long)(sleepLength - 1));

    // Update frame at regular intervals with slight randomness for natural feel
    if (currentMillis >= nextSleepFrameChange) {
        // Gradually move toward target index (creates smooth, natural closing)
        if (sleepIndex < targetIndex) {
            sleepIndex++;
        } else if (sleepIndex > targetIndex && esp_random() % 100 < 30) {
            // 30% chance to occasionally open slightly (natural sleep movement)
            sleepIndex--;
        }

        // Next frame change in 300-500ms (natural variation)
        nextSleepFrameChange = currentMillis + 300 + (esp_random() % 200);
    }

    display->drawEye(eyeSleepAnimation[sleepIndex]);
}

void EyeState::resetSleepFace() {
    sleepIndex = 0;
    startSleepTime = millis();
    nextSleepFrameChange = millis();
}

void EyeState::detransition() {
}

void EyeState::renderGooglyEye() {
    display->drawEye(eyeGoogly);
    googlyEye.renderEye(sensorData.accelZ, sensorData.accelX);
    display->drawEyePupil(eyePupil, googlyEye.x, googlyEye.y);
}
