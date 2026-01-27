#include "KMMXController.h"
#include <cmath>

void KMMXController::resetIdletime(KMMXController *controller) {
    controller->eyeState.playPrevState();
    controller->mouthState.resetMovingMouth();
    controller->mouthState.setSlowAnimation(false);
    controller->stillTime = 0;
    controller->isSleeping = false;
    controller->hornLED.setBrightness(controller->prevHornBright, 5);
    controller->accelerometer.setDataRate(LIS3DH_DATARATE_25_HZ);
}

void KMMXController::resetIdletime() {
    if (isSleeping) {
        eyeState.playPrevState();
        mouthState.resetMovingMouth();
        mouthState.setSlowAnimation(false);
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
    controller->mouthState.setSlowAnimation(true);
    controller->isSleeping = true;
    controller->accelerometer.setDataRate(LIS3DH_DATARATE_1_HZ);
}

void KMMXController::checkIdleAndSleep(KMMXController *controller, unsigned long currentTime) {
    float currentThreshold = controller->isSleeping ? sleepingAccThreshold : idleAccThreshold;
    if (controller->eyeState.getState() != EyeStateEnum::IDLE && controller->eyeState.getState() != EyeStateEnum::SLEEP) {
        return;
    }
    const SensorData& current = controller->sensorBuffer[controller->activeBuffer];
    const SensorData& prev = controller->prevSensorData;

    if (fabsf(current.accelX - prev.accelX) < currentThreshold &&
        fabsf(current.accelY - prev.accelY) < currentThreshold &&
        fabsf(current.accelZ - prev.accelZ) < currentThreshold) {
        if (controller->stillTime == 0) {
            controller->stillTime = currentTime;
        } else if (currentTime - controller->stillTime >= idleTimeout && !controller->isSleeping) {
            sleep(controller);
        }
    } else {
        resetIdletime(controller);
    }
}
