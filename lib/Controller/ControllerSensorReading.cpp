#include "Controller.h"
#include <Arduino.h>

void Controller::updateSensorValues() {
    proxSensor.read(&proxValue);
    sensorEvent = accSensor.getSensorEvent();
    prevX = lastX;
    prevY = lastY;
    prevZ = lastZ;
    lastX = sensorEvent->acceleration.x;
    lastY = sensorEvent->acceleration.y;
    lastZ = sensorEvent->acceleration.z;
}

void Controller::readSensor(void *parameter) {
    Controller *controller = static_cast<Controller *>(parameter);
    while (1) {
        unsigned long currentTime = millis();
        if (currentTime >= controller->nextRead) {
            controller->nextRead = currentTime + sensorUpdateInterval;
            controller->updateSensorValues();
            controller->checkIdleAndSleep(controller, currentTime);
        }
        controller->mouthState.getListEvent(*(controller->sensorEvent));
        controller->eyeState.getListEvent(*(controller->sensorEvent));
    }
}
