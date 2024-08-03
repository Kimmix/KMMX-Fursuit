#include "KMMXController.h"

void KMMXController::booping() {
    if (millis() >= nextBoop) {
        nextBoop = millis() + 100;
        boop.getBoop(proxValue, inRange, isBoop, boopSpeed, isContinuous, isAngry);
        if (isBoop) {
            fxState.setFlyingSpeed(boopSpeed);
            fxState.setState(FXStateEnum::Heart);
            eyeState.setState(EyeStateEnum::BOOP);
            mouthState.setState(MouthStateEnum::BOOP);
            resetIdletime();
        } else if (inRange) {
            mouthState.setState(MouthStateEnum::BOOP);
            if (isSleeping) {
                resetIdletime();
            }
        } else if (isContinuous) {
            eyeState.setState(EyeStateEnum::BOOP);
        } else if (isAngry) {
            nextBoop = millis() + 1500;
            eyeState.setState(EyeStateEnum::ANGRY);
            mouthState.setState(MouthStateEnum::ANGRYBOOP);
            resetIdletime();
            Serial.println("Angry!!!!");
            statusLED.setColor(255, 0, 0);
        }
    }
}
