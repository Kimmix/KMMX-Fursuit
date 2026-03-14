#include "KMMXController.h"

void KMMXController::handleBoop() {
    static bool lastIsAngry = false;
    static bool wasInBoopRange = false;
    static bool wasBooped = false;

    if (millis() >= nextBoop) {
        nextBoop = millis() + 50;
        boop.getBoop(sensorBuffer[activeBuffer].proximity, inBoopRange, isBooping, boopSpeed, isContinuousBoop, isAngry);
        if (isBooping) {
            fxState.setFlyingSpeed(boopSpeed);
            fxState.setState(FXStateEnum::Heart);
            // Only set BOOP state if not already in BOOP state
            // This prevents resetting the animation and causing jerky playback
            if (eyeState.getState() != EyeStateEnum::BOOP) {
                eyeState.setState(EyeStateEnum::BOOP);
            }
            if (mouthState.getState() != MouthStateEnum::BOOP) {
                mouthState.setState(MouthStateEnum::BOOP);
            }
            resetIdleTime();
            statusLED.setColor(Color::CYAN);
            wasBooped = true;
            wasInBoopRange = false;
        } else if (inBoopRange) {
            if (mouthState.getState() != MouthStateEnum::BOOP) {
                mouthState.setState(MouthStateEnum::BOOP);
            }
            if (isSleeping) {
                resetIdleTime();
            }
            statusLED.setColor(Color::LIGHT_PINK);
            wasInBoopRange = true;
        } else if (isContinuousBoop) {
            // Only set BOOP state if not already in BOOP state
            // This prevents resetting the auto-reset timer
            if (eyeState.getState() != EyeStateEnum::BOOP) {
                eyeState.setState(EyeStateEnum::BOOP);
            }
            statusLED.setColor(Color::PINK);
            wasBooped = true;
            wasInBoopRange = false;
        } else if (isAngry && !lastIsAngry) {
            nextBoop = millis() + 1500;
            eyeState.setState(EyeStateEnum::ANGRY);
            mouthState.setState(MouthStateEnum::ANGRYBOOP);
            resetIdleTime();
            statusLED.setColor(Color::RED);
            wasBooped = false;
            wasInBoopRange = false;
        } else {
            // No boop conditions active
            // If we were in range but didn't get booped, show sad eyes
            if (wasInBoopRange && !wasBooped) {
                if (eyeState.getState() != EyeStateEnum::SAD) {
                    eyeState.setState(EyeStateEnum::SAD);
                    statusLED.setColor(Color::BLUE);
                }
            }
            wasInBoopRange = false;
            wasBooped = false;
        }
        lastIsAngry = isAngry;
    }
}
