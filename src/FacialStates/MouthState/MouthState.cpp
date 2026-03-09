#include "MouthState.h"
#include "Utils/Utils.h"
#include <Arduino.h>

MouthState::MouthState(Hub75DMA* display) : display(display) {
    // Initialize wah animation (60 frames, bouncy/elastic feel)
    TimeBasedAnimation::init(wahAnim, mouthWahAnimation, wahLength, TimeBasedAnimation::CONFIG_WAH);

    // Initialize idle breathing animation (60 frames, slow breathing)
    TimeBasedAnimation::init(idleAnim, defaultAnimation, defaultAnimationLength, TimeBasedAnimation::CONFIG_BREATHING);

    // Initialize angry animation (20 frames, quick transition)
    TimeBasedAnimation::init(angryAnim, mouthAngryAnimation, angryLength, TimeBasedAnimation::CONFIG_TRANSITION);
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
            display->drawMouth(TimeBasedAnimation::update(wahAnim));
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
    if (newState == MouthStateEnum::ANGRYBOOP) {
        // Reset angry animation to start from beginning
        TimeBasedAnimation::reset(angryAnim);
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
    TimeBasedAnimation::reset(idleAnim);
}

void MouthState::setSlowAnimation(bool slow) {
    if (slow) {
        // Slow breathing: use preset
        TimeBasedAnimation::setConfig(idleAnim, TimeBasedAnimation::CONFIG_BREATHING_SLOW);
    } else {
        // Normal breathing: use preset
        TimeBasedAnimation::setConfig(idleAnim, TimeBasedAnimation::CONFIG_BREATHING);
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
    // Default animation using TimeBasedAnimation
    mouthFrame = TimeBasedAnimation::update(idleAnim);
}

void MouthState::angryBoop() {
    const uint8_t* currentFrame = TimeBasedAnimation::update(angryAnim);

    if (isTransitioning) {
        display->drawMouth(currentFrame);

        // Check if transition animation is complete
        if (TimeBasedAnimation::isComplete(angryAnim)) {
            isTransitioning = false;
        }
    } else {
        // Hold at frame near the end (angryLength - 8)
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
