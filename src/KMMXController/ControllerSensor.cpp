#include "KMMXController.h"

void KMMXController::setupSensors() {
    Wire.begin(S3_SDA, S3_SCL);
    statusLED.init();
    cheekPanel.configure(0xFF446C, 0xF9826C, 500, 2000);
    cheekPanel.init();
    accelerometer.setUp();
    boopInitialized = proximitySensor.setup();

    // Create sensor reading task on Core 0 (Core 1 is for Arduino loop/rendering)
    xTaskCreatePinnedToCore(readSensorTask, "SensorTask", 4096, this, 2, &sensorTaskHandle, 0);

    Serial.println("Sensor initialization complete");
}

void KMMXController::readSensorTask(void *parameter) {
    KMMXController *ctrl = static_cast<KMMXController *>(parameter);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    constexpr TickType_t xFrequency = pdMS_TO_TICKS(sensorUpdateInterval);

    while (true) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // Write to inactive buffer (double-buffer pattern)
        uint8_t writeBuffer = 1 - ctrl->activeBuffer;
        sensors_event_t* event = ctrl->accelerometer.getSensorEvent();

        // Update sensor data in inactive buffer
        ctrl->sensorBuffer[writeBuffer].accelX = event->acceleration.x;
        ctrl->sensorBuffer[writeBuffer].accelY = event->acceleration.y;
        ctrl->sensorBuffer[writeBuffer].accelZ = event->acceleration.z;
        ctrl->proximitySensor.read(&ctrl->sensorBuffer[writeBuffer].proximity);

        // Atomic swap - readers now use the new buffer
        ctrl->activeBuffer = writeBuffer;

        // Store previous values for idle detection
        ctrl->prevSensorData = ctrl->sensorBuffer[1 - writeBuffer];

        // Check idle/sleep state
        ctrl->checkIdleAndSleep(ctrl, millis());

        // Update facial states with new sensor data
        ctrl->mouthState.setSensorData(ctrl->sensorBuffer[ctrl->activeBuffer]);
        ctrl->eyeState.setSensorData(ctrl->sensorBuffer[ctrl->activeBuffer]);
    }
}

const SensorData& KMMXController::getSensorData() const {
    return sensorBuffer[activeBuffer];
}
