#include "MouthState.h"
#include "Utils/Utils.h"
#include <Arduino.h>

MouthState::MouthState(Hub75DMA* display) : display(display) {}

void MouthState::startMic() {
    viseme.initMic();
    startVisemeTask();
}

void MouthState::update() {
    updateAnimation();
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

void MouthState::getListEvent(const sensors_event_t& eventData) {
    event = eventData;
}

void MouthState::drawDefault() {
    display->drawMouth(mouthFrame);
}

void MouthState::movingMouth() {
    float yAcc = event.acceleration.x;
    const float upThreshold = -2.00, downThreshold = 3.00,
                upMaxThreshold = -7.00, downMaxThreshold = 8.00;
    if (yAcc < upThreshold) {
        int level = mapFloat(yAcc, upThreshold, upMaxThreshold, 0, 19);
        mouthFrame = mouthUp[level];
    } else if (yAcc > downThreshold) {
        int level = mapFloat(yAcc, downThreshold, downMaxThreshold, 0, 19);
        mouthFrame = mouthDown[level];
    } else {
        mouthFrame = defaultAnimation[defaultAnimationIndex];
    }
}

void MouthState::updateAnimation() {
    if (millis() >= mouthInterval) {
        updateIndex();
        mouthInterval = millis() + 80;
    }
}

void MouthState::updateIndex() {
    if (increasingIndex) {
        defaultAnimationIndex++;
        if (defaultAnimationIndex >= 19) {
            defaultAnimationIndex = 19;
            increasingIndex = false;
        }
    } else {
        defaultAnimationIndex--;
        if (defaultAnimationIndex <= 0) {
            defaultAnimationIndex = 0;
            increasingIndex = true;
        }
    }
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
