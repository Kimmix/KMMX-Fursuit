#include "KMMXController.h"

void KMMXController::setupSensors() {
    // Configure I2C pins with pullups
    pinMode(S3_SDA, INPUT_PULLUP);
    pinMode(S3_SCL, INPUT_PULLUP);
    // Initialize I2C with the correct pins
    Wire.begin(S3_SDA, S3_SCL);
    Wire.setClock(100000); // Lower I2C clock speed to 100kHz for reliability
    // For ESP32-S3, try to set timeout
    #if defined(ESP32)
        Wire.setTimeOut(1000); // 1000ms timeout
    #endif

    statusLED.init();
    cheekPanel.configure(0xFF446C, 0xF9826C, 500, 2000);
    cheekPanel.init();
    accSensor.setUp();
    initBoop = proxSensor.setup();

    // Create sensor reading task
    xTaskCreatePinnedToCore(readSensor, "SensorEventTask", 4096, this, 1, &sensorEventTaskHandle, 0);
    sensorEvent = new sensors_event_t();
    memset(sensorEvent, 0, sizeof(sensors_event_t));

    // Initial acceleration values
    lastX = lastY = lastZ = 0;
    prevX = prevY = prevZ = 0;

    Serial.println("Sensor initialization complete");
}

void KMMXController::updateSensorValues() {
    // Get proxSensor value
    proxSensor.read(&proxValue);
    sensorEvent = accSensor.getSensorEvent();
    // Store the current acceleration values
    // Only update prevX/Y/Z if lastX/Y/Z changed
    if (lastX != sensorEvent->acceleration.x) prevX = lastX;
    if (lastY != sensorEvent->acceleration.y) prevY = lastY;
    if (lastZ != sensorEvent->acceleration.z) prevZ = lastZ;

    // Update the last acceleration values
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
