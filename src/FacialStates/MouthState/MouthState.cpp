#include "MouthState.h"
#include "Utils/Utils.h"
#include <Arduino.h>

MouthState::MouthState(Hub75DMA* display) : display(display) {
    // Initialize wah animation with bouncy timing
    AnimationHelper::initAnimation(wahAnim, mouthWahAnimation, wahLength, AnimationHelper::TIMING_BOUNCY);
    // Initialize idle breathing animation
    AnimationHelper::initAnimation(idleAnim, defaultAnimation, defaultAnimationLength, AnimationHelper::TIMING_BREATHING);
}

void MouthState::startMic() {
    viseme.initMic();
    startVisemeTask();
}

void MouthState::update() {
    switch (currentState) {
        case MouthStateEnum::IDLE:
            movingMouth();
            drawDefault();
            break;
        case MouthStateEnum::BOOP:
            display->drawMouth(mouthOH15);
            if (millis() - resetBoop >= 700) {
                currentState = prevState;
            }
            break;
        case MouthStateEnum::ANGRYBOOP:
            angryBoop();
            if (millis() - resetBoop >= 1500) {
                currentState = prevState;
            }
            break;
        case MouthStateEnum::TALKING:
            if (visemeFrame == mouthDefault) {
                drawDefault();
            } else {
                display->drawMouth(visemeFrame);
            }
            break;
        case MouthStateEnum::WAH:
            display->drawMouth(AnimationHelper::updateAnimation(wahAnim));
            break;
        default:
            break;
    }
}

void MouthState::setState(MouthStateEnum newState) {
    savePrevState(currentState);
    if (newState == MouthStateEnum::BOOP || newState == MouthStateEnum::ANGRYBOOP) {
        resetBoop = millis();
    }
    if (currentState != newState) {
        isTransitioning = true;
        currentState = newState;
    }
}

void MouthState::savePrevState(MouthStateEnum newState) {
    if (newState == MouthStateEnum::BOOP || newState == MouthStateEnum::ANGRYBOOP) {
        return;
    }
    prevState = newState;
}

MouthStateEnum MouthState::getState() const {
    return currentState;
}

void MouthState::setSensorData(const SensorData& data) {
    sensorData = data;
}

void MouthState::drawDefault() {
    display->drawMouth(mouthFrame);
}

void MouthState::resetMovingMouth() {
    idleAnim.index = 0;
    idleAnim.increasing = true;
}

void MouthState::setSlowAnimation(bool slow) {
    if (slow) {
        AnimationHelper::setTiming(idleAnim, AnimationHelper::TIMING_BREATHING_SLOW);
    } else {
        AnimationHelper::setTiming(idleAnim, AnimationHelper::TIMING_BREATHING);
    }
}

void MouthState::movingMouth() {
    float yAcc = sensorData.accelX;
    const float upThreshold = -2.00, downThreshold = 3.00,
                upMaxThreshold = -7.00, downMaxThreshold = 8.00;
    static float smoothedUpAcc = 0.0f;
    static float smoothedDownAcc = 0.0f;
    // Check for upward movement (negative direction)
    int upLevel = smoothAccelerometerMovement(yAcc, smoothedUpAcc, upThreshold, upMaxThreshold, 0.3f, 0.5f, 59, true);
    if (upLevel >= 0) {
        mouthFrame = mouthUp[upLevel];
        return;
    }
    // Check for downward movement (positive direction)
    int downLevel = smoothAccelerometerMovement(yAcc, smoothedDownAcc, downThreshold, downMaxThreshold, 0.3f, 0.5f, 59, false);
    if (downLevel >= 0) {
        mouthFrame = mouthDown[downLevel];
        return;
    }
    // Default animation using AnimationHelper
    mouthFrame = AnimationHelper::updateAnimation(idleAnim);
}

void MouthState::angryBoop() {
    if (isTransitioning) {
        display->drawMouth(mouthAngryAnimation[angryIndex]);
        if (millis() >= nextAngry) {
            nextAngry = millis() + 7;
            angryIndex++;
            if (angryIndex == angryLength) {
                angryIndex = 0;
                isTransitioning = false;
            }
        }
    } else {
        display->drawMouth(mouthAngryAnimation[angryLength - 8]);
    }
}

void MouthState::startVisemeTask() {
    Serial.println(F("Start viseme task..."));
    xTaskCreatePinnedToCore(visemeRenderingTask, "VisemeTask", 2048, this, 2, &visemeTaskHandle, 0);
}

void MouthState::visemeRenderingTask(void* parameter) {
    MouthState* mouthState = reinterpret_cast<MouthState*>(parameter);
    while (true) {
        mouthState->visemeFrame = mouthState->viseme.renderViseme();
    }
}
