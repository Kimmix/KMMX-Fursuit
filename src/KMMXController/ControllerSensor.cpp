#include "KMMXController.h"

void KMMXController::setupSensors() {
    accSensor.setUp();
    initBoop = proxSensor.setup();
    xTaskCreatePinnedToCore(readSensor, "SensorEventTask", 4096, this, 1, &sensorEventTaskHandle, 0);
}

void KMMXController::updateSensorValues() {
    proxSensor.read(&proxValue);
    sensorEvent = accSensor.getSensorEvent();
    prevX = lastX;
    prevY = lastY;
    prevZ = lastZ;
    lastX = sensorEvent->acceleration.x;
    lastY = sensorEvent->acceleration.y;
    lastZ = sensorEvent->acceleration.z;
}

void KMMXController::readSensor(void *parameter) {
    KMMXController *controller = static_cast<KMMXController *>(parameter);
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
