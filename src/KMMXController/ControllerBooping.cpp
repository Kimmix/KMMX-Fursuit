#include "KMMXController.h"

void KMMXController::handleBoop() {
    #if HAS_PROXIMITY
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
                eyeState.setState(EyeStateEnum::BOOP, false, 2500);  // Temporary, 2.5s timeout
            }
            if (mouthState.getState() != MouthStateEnum::BOOP) {
                mouthState.setState(MouthStateEnum::BOOP, false, 700);  // Temporary, 700ms timeout
            }
            resetIdleTime();
            #if HAS_CHEEK_PANEL
            statusLED.setColor(Color::CYAN);
            #endif
            wasBooped = true;
            wasInBoopRange = false;
        } else if (inBoopRange) {
            if (mouthState.getState() != MouthStateEnum::BOOP) {
                mouthState.setState(MouthStateEnum::BOOP, false, 700);  // Temporary, 700ms timeout
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
                eyeState.setState(EyeStateEnum::BOOP, false, 2500);  // Temporary, 2.5s timeout
            }
            statusLED.setColor(Color::PINK);
            wasBooped = true;
            wasInBoopRange = false;
        } else if (isAngry && !lastIsAngry) {
            nextBoop = millis() + 1500;
            eyeState.setState(EyeStateEnum::ANGRY, false, 1500);  // Temporary, 1.5s timeout
            mouthState.setState(MouthStateEnum::ANGRYBOOP, false, 1500);  // Temporary, 1.5s timeout
            resetIdleTime();
            statusLED.setColor(Color::RED);
            wasBooped = false;
            wasInBoopRange = false;
        } else {
            // No boop conditions active
            // If we were in range but didn't get booped, show sad eyes
            if (wasInBoopRange && !wasBooped) {
                if (eyeState.getState() != EyeStateEnum::SAD) {
                    eyeState.setState(EyeStateEnum::SAD, false, 3000);  // Temporary, 3s timeout
                    statusLED.setColor(Color::BLUE);
                }
            }
            wasInBoopRange = false;
            wasBooped = false;
        }
        lastIsAngry = isAngry;
    }
    #endif  // HAS_PROXIMITY
}
