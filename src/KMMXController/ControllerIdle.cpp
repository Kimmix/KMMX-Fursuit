#include "KMMXController.h"

void KMMXController::resetIdletime(KMMXController *controller) {
    controller->eyeState.playPrevState();
    controller->stillTime = 0;
    controller->isSleeping = false;
    controller->hornLED.setBrightness(controller->prevHornBright, 5);
}

void KMMXController::resetIdletime() {
    if (isSleeping) {
        eyeState.playPrevState();
    }
    stillTime = 0;
    isSleeping = false;
    hornLED.setBrightness(prevHornBright, 5);
}

void KMMXController::sleep(KMMXController *controller) {
    controller->prevHornBright = controller->hornLED.getBrightness();
    controller->hornLED.setBrightness(5);
    controller->eyeState.setState(EyeStateEnum::SLEEP);
    controller->isSleeping = true;
}

void KMMXController::checkIdleAndSleep(KMMXController *controller, unsigned long currentTime) {
    if (controller->eyeState.getState() != EyeStateEnum::IDLE) {
        return;
    }
    if (abs(lastX - prevX) < idleAccThreshold &&
        abs(lastY - prevY) < idleAccThreshold &&
        abs(lastZ - prevZ) < idleAccThreshold) {
        if (controller->stillTime == 0) {
            controller->stillTime = currentTime;
        } else if (currentTime - stillTime >= idleTimeout && !isSleeping) {
            sleep(controller);
        }
    } else {
        resetIdletime(controller);
    }
}
