#include "Controller.h"

void Controller::setupSensors() {
    accSensor.setUp();
    proxSensor.setup();
    xTaskCreatePinnedToCore(readSensor, "SensorEventTask", 4096, this, 1, &sensorEventTaskHandle, 0);
}
