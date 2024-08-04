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
            statusLED.setColor(Color::CYAN);
        } else if (inRange) {
            mouthState.setState(MouthStateEnum::BOOP);
            if (isSleeping) {
                resetIdletime();
            }
            statusLED.setColor(Color::LIGHT_PINK);
        } else if (isContinuous) {
            eyeState.setState(EyeStateEnum::BOOP);
            statusLED.setColor(Color::PINK);
        } else if (isAngry) {
            nextBoop = millis() + 1500;
            eyeState.setState(EyeStateEnum::ANGRY);
            mouthState.setState(MouthStateEnum::ANGRYBOOP);
            resetIdletime();
            Serial.println("Angry!!!!");
            statusLED.setColor(Color::RED);
        }
    }
}
