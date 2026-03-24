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

    // Initialize animations with transition + loop pattern
    // Eh: no loop (plays once)
    initAnimationData(ehData, mouthEhAnimation, ehLength, 0, TimeBasedAnimation::CONFIG_ANTICIPATION);

    // Pout: loop last 10 frames
    initAnimationData(poutData, mouthPoutAnimation, poutLength, 10, TimeBasedAnimation::CONFIG_SMOOTH_LOOP);

    // Drooling: loop last 20 frames
    initAnimationData(droolingData, mouthDroolingAnimation, droolingLength, 20, TimeBasedAnimation::CONFIG_BREATHING);
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
        case MouthStateEnum::EH:
            playAnimationWithLoop(ehData);
            break;
        case MouthStateEnum::POUT:
            playAnimationWithLoop(poutData);
            break;
        case MouthStateEnum::DROOLING:
            playAnimationWithLoop(droolingData);
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
    if (newState == MouthStateEnum::EH) {
        // Reset Eh animation to start from beginning
        resetAnimation(ehData);
    }
    if (newState == MouthStateEnum::POUT) {
        // Reset Pout animation to start from beginning
        resetAnimation(poutData);
    }
    if (newState == MouthStateEnum::DROOLING) {
        // Reset Drooling animation to start from beginning
        resetAnimation(droolingData);
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
    // Removed accelerometer-based tilt movement - replaced by motion detection system
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

void MouthState::initAnimationData(MouthAnimationData& data, const uint8_t** frames, uint8_t frameCount,
                                   uint8_t loopFrameCount, const TimeBasedAnimConfig& loopConfig) {
    data.frames = frames;
    data.frameCount = frameCount;
    data.loopFrameCount = loopFrameCount;

    // Initialize transition animation (full animation, plays once)
    TimeBasedAnimation::init(data.transitionAnim, frames, frameCount, TimeBasedAnimation::CONFIG_TRANSITION);

    // Initialize loop animation (last N frames only)
    // If loopFrameCount is 0, no loop (animation plays once and holds on last frame)
    if (loopFrameCount > 0) {
        // Uses pointer arithmetic to reference the last N frames without duplicating data
        TimeBasedAnimation::init(data.loopAnim, &frames[frameCount - loopFrameCount], loopFrameCount, loopConfig);
    }
}

void MouthState::resetAnimation(MouthAnimationData& data) {
    TimeBasedAnimation::reset(data.transitionAnim);
    if (data.loopFrameCount > 0) {
        TimeBasedAnimation::reset(data.loopAnim);
    }
}

void MouthState::playAnimationWithLoop(MouthAnimationData& animData) {
    const uint8_t* currentFrame;

    if (isTransitioning) {
        // During transition: use full animation
        currentFrame = TimeBasedAnimation::update(animData.transitionAnim);

        // Also update loop animation in the background so it's in sync when we switch
        if (animData.loopFrameCount > 0) {
            TimeBasedAnimation::update(animData.loopAnim);
        }

        // Check if transition animation is complete
        if (TimeBasedAnimation::isComplete(animData.transitionAnim)) {
            isTransitioning = false;
            // Loop animation is already running in sync, no reset needed
        }
    } else {
        // After transition: loop only the last N frames (or hold on last frame if no loop)
        if (animData.loopFrameCount > 0) {
            currentFrame = TimeBasedAnimation::update(animData.loopAnim);
        } else {
            // No loop - hold on last frame
            currentFrame = animData.frames[animData.frameCount - 1];
        }
    }

    display->drawMouth(currentFrame);
}
