#include "KMMXController.h"

void KMMXController::handleBoop() {
    static bool lastIsAngry = false;
    if (millis() >= nextBoop) {
        nextBoop = millis() + 50;
        boop.getBoop(sensorBuffer[activeBuffer].proximity, inBoopRange, isBooping, boopSpeed, isContinuousBoop, isAngry);
        if (isBooping) {
            fxState.setFlyingSpeed(boopSpeed);
            fxState.setState(FXStateEnum::Heart);
            eyeState.setState(EyeStateEnum::BOOP);
            mouthState.setState(MouthStateEnum::BOOP);
            resetIdleTime();
            statusLED.setColor(Color::CYAN);
        } else if (inBoopRange) {
            mouthState.setState(MouthStateEnum::BOOP);
            if (isSleeping) {
                resetIdleTime();
            }
            statusLED.setColor(Color::LIGHT_PINK);
        } else if (isContinuousBoop) {
            eyeState.setState(EyeStateEnum::BOOP);
            statusLED.setColor(Color::PINK);
        } else if (isAngry && !lastIsAngry) {
            nextBoop = millis() + 1500;
            eyeState.setState(EyeStateEnum::ANGRY);
            mouthState.setState(MouthStateEnum::ANGRYBOOP);
            resetIdleTime();
            statusLED.setColor(Color::RED);
        }
        lastIsAngry = isAngry;
    }
}
