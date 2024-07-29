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
    if (abs(lastX - prevX) < idleAccThreshold &&
        abs(lastY - prevY) < idleAccThreshold &&
        abs(lastZ - prevZ) < idleAccThreshold) {
        if (controller->stillTime == 0) {
            controller->stillTime = currentTime;
        } else if (currentTime - stillTime >= 30000 && !isSleeping) {
            sleep(controller);
        }
    } else {
        resetIdletime(controller);
    }
}
