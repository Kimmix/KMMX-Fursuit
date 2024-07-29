#include "KMMXController.h"

void KMMXController::resetIdletime(KMMXController *controller) {
    controller->eyeState.playPrevState();
    controller->stillTime = 0;
    controller->isSleeping = false;
}

void KMMXController::resetIdletime() {
    stillTime = 0;
    isSleeping = false;
}

void KMMXController::sleep(KMMXController *controller) {
    controller->eyeState.setState(EyeStateEnum::SLEEP);
    controller->isSleeping = true;
}

void KMMXController::checkIdleAndSleep(KMMXController *controller, unsigned long currentTime) {
    // Check if the accelerometer values indicate stillness
    float deltaX = abs(controller->lastX - controller->prevX);
    float deltaY = abs(controller->lastY - controller->prevY);
    float deltaZ = abs(controller->lastZ - controller->prevZ);

    // if (deltaX < controller->THRESHOLD && deltaY < controller->THRESHOLD && deltaZ < controller->THRESHOLD) {
    //     if (controller->stillTime == 0) {
    //         controller->stillTime = currentTime;
    //     } else if (currentTime - controller->stillTime > 5000 && !controller->isSleeping) {  // 5 seconds
    //         controller->sleep(controller);
    //     }
    // } else {
    //     controller->resetIdletime();
    // }
}
