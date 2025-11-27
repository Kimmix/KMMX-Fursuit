#include "KMMXController.h"
#include <cmath>

void KMMXController::setupSensors() {
    // Configure I2C pins with pullups
    // pinMode(S3_SDA, INPUT_PULLUP);
    // pinMode(S3_SCL, INPUT_PULLUP);
    Wire.begin(S3_SDA, S3_SCL);
    statusLED.init();
    cheekPanel.configure(0xFF446C, 0xF9826C, 500, 2000);
    cheekPanel.init();
    accelerometer.setUp();
    boopInitialized = proximitySensor.setup();

    // Allocate sensor event struct
    sensorEvent = new sensors_event_t();
    if (sensorEvent) {
        memset(sensorEvent, 0, sizeof(sensors_event_t));
    }

    // Initial acceleration values
    lastAccelX = lastAccelY = lastAccelZ = 0;
    prevAccelX = prevAccelY = prevAccelZ = 0;

    // Create sensor reading task with appropriate priority
    xTaskCreatePinnedToCore(readSensorTask, "SensorTask", 4096, this, 2, &sensorTaskHandle, 0);

    Serial.println("Sensor initialization complete");
}

void KMMXController::updateSensorValues() {
    // Read proximity sensor
    proximitySensor.read(&proximityValue);
    sensorEvent = accelerometer.getSensorEvent();

    if (!sensorEvent) return; // Safety check

    // Use a small epsilon for float comparison
    constexpr float epsilon = 0.001f;

    // Store previous values only if significantly changed
    if (fabsf(lastAccelX - sensorEvent->acceleration.x) > epsilon) prevAccelX = lastAccelX;
    if (fabsf(lastAccelY - sensorEvent->acceleration.y) > epsilon) prevAccelY = lastAccelY;
    if (fabsf(lastAccelZ - sensorEvent->acceleration.z) > epsilon) prevAccelZ = lastAccelZ;

    // Update the last acceleration values
    lastAccelX = sensorEvent->acceleration.x;
    lastAccelY = sensorEvent->acceleration.y;
    lastAccelZ = sensorEvent->acceleration.z;
}

void KMMXController::readSensorTask(void *parameter) {
    KMMXController *controller = static_cast<KMMXController *>(parameter);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(sensorUpdateInterval);

    while (1) {
        // Use vTaskDelayUntil for consistent timing and CPU yielding
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        unsigned long currentTime = millis();
        controller->updateSensorValues();
        controller->checkIdleAndSleep(controller, currentTime);

        // Process events if sensorEvent is valid
        if (controller->sensorEvent) {
            controller->mouthState.getListEvent(*(controller->sensorEvent));
            controller->eyeState.getListEvent(*(controller->sensorEvent));
        }
    }
}
