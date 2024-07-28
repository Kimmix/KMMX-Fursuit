#include "Controller.h"
#include <Arduino.h>

void Controller::resetIdletime(Controller *controller) {
    controller->eyeState.playPrevState();
    controller->stillTime = 0;
    controller->isSleeping = false;
}

void Controller::resetIdletime() {
    stillTime = 0;
    isSleeping = false;
}

void Controller::sleep(Controller *controller) {
    controller->eyeState.setState(EyeStateEnum::SLEEP);
    controller->isSleeping = true;
}

void Controller::checkIdleAndSleep(Controller *controller, unsigned long currentTime) {
    // Check if the accelerometer values indicate stillness
    float deltaX = abs(controller->lastX - controller->prevX);
    float deltaY = abs(controller->lastY - controller->prevY);
    float deltaZ = abs(controller->lastZ - controller->prevZ);

    if (deltaX < controller->THRESHOLD && deltaY < controller->THRESHOLD && deltaZ < controller->THRESHOLD) {
        if (controller->stillTime == 0) {
            controller->stillTime = currentTime;
        } else if (currentTime - controller->stillTime > 5000 && !controller->isSleeping) {  // 5 seconds
            controller->sleep(controller);
        }
    } else {
        controller->resetIdletime();
    }
}
