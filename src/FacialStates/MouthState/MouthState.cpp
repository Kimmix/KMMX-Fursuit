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
    static float smoothedUpAcc = 0.0f;
    static float smoothedDownAcc = 0.0f;
    // Check for upward movement (negative direction)
    int upLevel = smoothAccelerometerMovement(yAcc, smoothedUpAcc, upThreshold, upMaxThreshold, 0.3f, 0.5f, 19, true);
    if (upLevel >= 0) {
        mouthFrame = mouthUp[upLevel];
        return;
    }
    // Check for downward movement (positive direction)
    int downLevel = smoothAccelerometerMovement(yAcc, smoothedDownAcc, downThreshold, downMaxThreshold, 0.3f, 0.5f, 19, false);
    if (downLevel >= 0) {
        mouthFrame = mouthDown[downLevel];
        return;
    }
    // Default animation when no significant movement
    mouthFrame = defaultAnimation[defaultAnimationIndex];
}

void MouthState::updateAnimation() {
    if (millis() >= mouthInterval) {
        updateIndex();
        int baseDelay = 80;
        int phaseVariance = getAnimationPhaseVariance();
        int randomVariance = (esp_random() % 31) - 15; // ±15ms randomness
        if (defaultAnimationIndex == 0 || defaultAnimationIndex == 19) {
            baseDelay += 100 + (esp_random() % 200); // Pause at extremes
        }
        mouthInterval = millis() + baseDelay + phaseVariance + randomVariance;
    }
}

void MouthState::updateIndex() {
    static bool shouldPause = false;
    static unsigned long pauseEnd = 0;
    if (shouldPause) {
        if (millis() < pauseEnd) return;
        shouldPause = false;
    }
    // Variable step size for more organic movement
    int step = (esp_random() % 100 < 20) ? 2 : 1; // 20% chance of double step
    if (increasingIndex) {
        defaultAnimationIndex += step;
        if (defaultAnimationIndex >= 19) {
            defaultAnimationIndex = 19;
            increasingIndex = false;
            // Random pause at peak (40% chance)
            if (esp_random() % 100 < 40) {
                shouldPause = true;
                pauseEnd = millis() + (esp_random() % 250 + 150);
            }
        }
    } else {
        defaultAnimationIndex -= step;
        if (defaultAnimationIndex <= 0) {
            defaultAnimationIndex = 0;
            increasingIndex = true;
            // Random pause at rest (60% chance, longer)
            if (esp_random() % 100 < 60) {
                shouldPause = true;
                pauseEnd = millis() + (esp_random() % 400 + 200);
            }
        }
    }
}

int MouthState::getAnimationPhaseVariance() {
    float progress = defaultAnimationIndex / 19.0f;
    if (increasingIndex) {
        // Inhale: slower at start and end, faster in middle
        if (progress < 0.4f) {
            return 15; // Slower start
        } else if (progress < 0.8f) {
            return -8; // Faster middle
        } else {
            return 12; // Slower end
        }
    } else {
        // Exhale: more consistent, slightly faster overall
        return -5 + (int)(progress * 8); // -5 to 3ms variance
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
