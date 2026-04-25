#include "KMMXController.h"
#include <cmath>
#include "esp_dsp.h"

void KMMXController::setupSensors() {
    Serial.println("=== Board Configuration ===");
    Serial.printf("Board: %s\n", BOARD_NAME);

    // Initialize I2C if board supports it
    #if HAS_I2C
    Wire.begin(S3_SDA, S3_SCL);
    Serial.println("I2C initialized");
    #endif

    // Initialize cheek panel and status LED if available
    #if HAS_CHEEK_PANEL
    statusLED.init();
    cheekPanel.configure(sideColor1RGB, sideColor2RGB, sideLEDFadeInterval, sideLEDPositionChangeDelay);
    cheekPanel.init();
    Serial.println("Cheek panel and status LED initialized");
    #endif

    // Initialize sensors and track their status
    Serial.println("=== Sensor Initialization ===");

    #if HAS_ACCELEROMETER
    accelerometerInitialized = accelerometer.setUp();
    Serial.printf("Accelerometer (LIS3DH): %s\n", accelerometerInitialized ? "OK" : "FAILED");
    #else
    Serial.println("Accelerometer: DISABLED (not available on this board)");
    #endif

    #if HAS_PROXIMITY
    boopInitialized = proximitySensor.setup();
    Serial.printf("Proximity (APDS9930): %s\n", boopInitialized ? "OK" : "FAILED");
    #else
    Serial.println("Proximity: DISABLED (not available on this board)");
    #endif

    #if HAS_HORN_LED
    Serial.println("Horn LED: OK");
    #else
    Serial.println("Horn LED: DISABLED (not available on this board)");
    #endif

    #if HAS_ACCELEROMETER || HAS_PROXIMITY
    // Check if any enabled sensors failed
    bool anyFailed = false;
    #if HAS_ACCELEROMETER
    if (!accelerometerInitialized) anyFailed = true;
    #endif
    #if HAS_PROXIMITY
    if (!boopInitialized) anyFailed = true;
    #endif

    if (anyFailed) {
        Serial.println("WARNING: One or more sensors failed to initialize. System will continue with reduced functionality.");
    }
    #endif

    // Ensure eye state starts at IDLE (prevents lingering states from previous sessions)
    eyeState.setState(EyeStateEnum::IDLE, true, 0);  // Persistent, no timeout (initial state)
    mouthState.setState(MouthStateEnum::IDLE, true, 0);  // Persistent, no timeout (initial state)

    // Set motion detection start time (current time + startup delay)
    motionDetectionStartTime = millis() + motionDetectionStartupDelay;

    // Create sensor reading task on Core 0
    xTaskCreatePinnedToCore(readSensorTask, "SensorTask", 4096, this, 2, &sensorTaskHandle, 0);

    // Create rendering task on Core 0 (keeps all heavy processing on one core)
    // Priority 1 is lower than SensorTask (priority 2), so sensors get priority
    xTaskCreatePinnedToCore(renderTask, "RenderTask", 4096, this, 1, &renderTaskHandle, 0);

    Serial.println("Sensor and render task initialization complete");
    Serial.printf("Motion detection will start in %d ms\n", motionDetectionStartupDelay);
}

void KMMXController::readSensorTask(void *parameter) {
    KMMXController *ctrl = static_cast<KMMXController *>(parameter);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    constexpr TickType_t xFrequency = pdMS_TO_TICKS(sensorUpdateInterval);

    while (true) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // Write to inactive buffer (double-buffer pattern)
        uint8_t writeBuffer = 1 - ctrl->activeBuffer;

        // Read accelerometer data if available
        #if HAS_ACCELEROMETER
        sensors_event_t* event = ctrl->accelerometer.getSensorEvent();
        #else
        // Provide default values when accelerometer is not present
        static sensors_event_t dummyEvent;
        dummyEvent.acceleration.x = 0.0f;
        dummyEvent.acceleration.y = -9.8f;  // Default gravity when upright
        dummyEvent.acceleration.z = 0.0f;
        sensors_event_t* event = &dummyEvent;
        #endif

        // Update sensor data in inactive buffer
        ctrl->sensorBuffer[writeBuffer].accelX = event->acceleration.x;
        ctrl->sensorBuffer[writeBuffer].accelY = event->acceleration.y;
        ctrl->sensorBuffer[writeBuffer].accelZ = event->acceleration.z;

        // Calculate acceleration magnitude using ESP-DSP optimized functions
        // Create vector for dot product calculation
        float accelVec[3] = {
            ctrl->sensorBuffer[writeBuffer].accelX,
            ctrl->sensorBuffer[writeBuffer].accelY,
            ctrl->sensorBuffer[writeBuffer].accelZ
        };

        // Use ESP-DSP dot product (hardware-accelerated on ESP32-S3)
        // magnitude = sqrt(x*x + y*y + z*z) = sqrt(dotprod(vec, vec))
        float dotProduct;
        dsps_dotprod_f32(accelVec, accelVec, &dotProduct, 3);
        ctrl->sensorBuffer[writeBuffer].accelMagnitude = sqrtf(dotProduct);

        // Read proximity sensor if available
        #if HAS_PROXIMITY
        ctrl->proximitySensor.read(&ctrl->sensorBuffer[writeBuffer].proximity);
        #else
        ctrl->sensorBuffer[writeBuffer].proximity = 0;
        #endif

        // Atomic swap - readers now use the new buffer
        ctrl->activeBuffer = writeBuffer;

        // Store previous values for idle detection
        ctrl->prevSensorData = ctrl->sensorBuffer[1 - writeBuffer];

        // Only run motion detection if accelerometer is available and initialized successfully
        #if HAS_ACCELEROMETER
        if (ctrl->accelerometerInitialized) {
            // Check idle/sleep state
            ctrl->checkIdleAndSleep(ctrl, millis());

            // Check motion detection features (shake, tilt, bounce, spin, petting)
            ctrl->checkMotionFeatures(ctrl);
        }
        #endif

        // Update facial states with new sensor data
        ctrl->mouthState.setSensorData(ctrl->sensorBuffer[ctrl->activeBuffer]);
        ctrl->eyeState.setSensorData(ctrl->sensorBuffer[ctrl->activeBuffer]);
    }
}

const SensorData& KMMXController::getSensorData() const {
    return sensorBuffer[activeBuffer];
}
