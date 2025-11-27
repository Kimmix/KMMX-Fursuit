#include "KMMXController.h"

void KMMXController::booping() {
    static bool lastIsAngry = false;
    if (millis() >= nextBoop) {
        nextBoop = millis() + 50;
        boop.getBoop(proximityValue, inBoopRange, isBooping, boopSpeed, isContinuousBoop, isAngry);
        if (isBooping) {
            fxState.setFlyingSpeed(boopSpeed);
            fxState.setState(FXStateEnum::Heart);
            eyeState.setState(EyeStateEnum::BOOP);
            mouthState.setState(MouthStateEnum::BOOP);
            resetIdletime();
            statusLED.setColor(Color::CYAN);
        } else if (inBoopRange) {
            mouthState.setState(MouthStateEnum::BOOP);
            if (isSleeping) {
                resetIdletime();
            }
            statusLED.setColor(Color::LIGHT_PINK);
        } else if (isContinuousBoop) {
            eyeState.setState(EyeStateEnum::BOOP);
            statusLED.setColor(Color::PINK);
        } else if (isAngry && !lastIsAngry) {
            nextBoop = millis() + 1500;
            eyeState.setState(EyeStateEnum::ANGRY);
            mouthState.setState(MouthStateEnum::ANGRYBOOP);
            resetIdletime();
            statusLED.setColor(Color::RED);
        }
        lastIsAngry = isAngry;
    }
}
