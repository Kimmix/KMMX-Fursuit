#include "KMMXController.h"

void KMMXController::resetIdletime(KMMXController *controller) {
    controller->eyeState.playPrevState();
    controller->stillTime = 0;
    controller->isSleeping = false;
    controller->hornLED.setBrightness(controller->prevHornBright, 5);
    controller->accelerometer.setDataRate(LIS3DH_DATARATE_25_HZ);
}

void KMMXController::resetIdletime() {
    if (isSleeping) {
        eyeState.playPrevState();
    }
    stillTime = 0;
    isSleeping = false;
    hornLED.setBrightness(prevHornBright, 5);
    accelerometer.setDataRate(LIS3DH_DATARATE_25_HZ);
}

void KMMXController::sleep(KMMXController *controller) {
    controller->prevHornBright = controller->hornLED.getBrightness();
    controller->hornLED.setBrightness(5);
    controller->eyeState.setState(EyeStateEnum::SLEEP);
    controller->isSleeping = true;
    controller->accelerometer.setDataRate(LIS3DH_DATARATE_1_HZ);
}

void KMMXController::checkIdleAndSleep(KMMXController *controller, unsigned long currentTime) {
    float currentThreshold = controller->isSleeping ? sleepingAccThreshold : idleAccThreshold;
    if (controller->eyeState.getState() != EyeStateEnum::IDLE && controller->eyeState.getState() != EyeStateEnum::SLEEP) {
        return;
    }
    if (abs(lastAccelX - prevAccelX) < currentThreshold &&
        abs(lastAccelY - prevAccelY) < currentThreshold &&
        abs(lastAccelZ - prevAccelZ) < currentThreshold) {
        if (controller->stillTime == 0) {
            controller->stillTime = currentTime;
        } else if (currentTime - stillTime >= idleTimeout && !isSleeping) {
            sleep(controller);
        }
    } else {
        resetIdletime(controller);
    }
}
