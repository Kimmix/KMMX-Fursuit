#include "FXState.h"
#include <Arduino.h>

FXState::FXState(Hub75DMA* displayPtr) : display(displayPtr), flyingHeart(displayPtr) {}

void FXState::update() {
    switch (currentState) {
        case FXStateEnum::IDLE:
            break;
        case FXStateEnum::Heart:
            flyingHeartState();
            break;
        case FXStateEnum::Blush:
            break;
        default:
            currentState = FXStateEnum::IDLE;
            break;
    }
}

void FXState::setState(FXStateEnum newState) {
    if (newState == FXStateEnum::Heart) {
        resetHeart = millis();
        flyingHeart.reset();
    }
    currentState = newState;
}

FXStateEnum FXState::getState() const {
    return currentState;
}

void FXState::flyingHeartState() {
    flyingHeart.renderHeart();
    if (millis() - resetHeart >= 7000) {
        currentState = FXStateEnum::IDLE;
        flyingHeart.resetAll();
    }
}

void FXState::setFlyingSpeed(float i) {
    flyingHeart.setSpeed(i);
}
