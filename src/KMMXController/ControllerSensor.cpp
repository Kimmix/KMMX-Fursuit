#include "KMMXController.h"
#include <cmath>
#include "esp_dsp.h"
#include "Devices/Proximity/VL6180XSensor.h"
#include "Devices/Proximity/APDS9930Sensor.h"

// Destructor - clean up dynamically allocated proximity sensor
KMMXController::~KMMXController() {
    if (proximitySensor != nullptr) {
        delete proximitySensor;
        proximitySensor = nullptr;
    }
}

void KMMXController::setupSensors() {
    Wire.begin(S3_SDA, S3_SCL);
    // Wire.setClock(400000);  // Set I2C to 400kHz Fast Mode (default is 100kHz) for faster sensor reads
    statusLED.init();
    cheekPanel.configure(sideColor1RGB, sideColor2RGB, sideLEDFadeInterval, sideLEDPositionChangeDelay);
    cheekPanel.init();

    // Initialize sensors and track their status
    accelerometerInitialized = accelerometer.setUp();

    // Auto-detect proximity sensor (try VL6180X first, fallback to APDS9930)
    Serial.println("=== Proximity Sensor Detection ===");

    // Try VL6180X first (priority sensor)
    Serial.println("Attempting VL6180X initialization...");
    VL6180XSensor* vl6180x = new VL6180XSensor();
    if (vl6180x->setup()) {
        proximitySensor = vl6180x;
        boopInitialized = true;
        Serial.println("VL6180X detected and initialized successfully!");
    } else {
        // VL6180X failed, try APDS9930
        delete vl6180x;
        Serial.println("VL6180X not found, trying APDS9930...");

        APDS9930Sensor* apds = new APDS9930Sensor();
        if (apds->setup()) {
            proximitySensor = apds;
            boopInitialized = true;
            Serial.println("APDS9930 detected and initialized successfully!");
        } else {
            // Both sensors failed
            delete apds;
            proximitySensor = nullptr;
            boopInitialized = false;
            Serial.println("WARNING: No proximity sensor detected!");
        }
    }

    oledInitialized = oledDisplay.setup();

    // Print sensor initialization status
    Serial.println("=== Sensor Initialization Summary ===");
    Serial.printf("Accelerometer (LIS3DH): %s\n", accelerometerInitialized ? "OK" : "FAILED");
    Serial.printf("Proximity Sensor: %s\n", boopInitialized ? "OK" : "FAILED");
    Serial.printf("OLED Display (SSD1306): %s\n", oledInitialized ? "OK" : "FAILED");
    if (!accelerometerInitialized || !boopInitialized) {
        Serial.println("WARNING: One or more sensors failed to initialize. System will continue with reduced functionality.");
    }

    // Ensure eye state starts at IDLE (prevents lingering states from previous sessions)
    eyeState.setState(EyeStateEnum::IDLE, true, 0);  // Persistent, no timeout (initial state)
    mouthState.setState(MouthStateEnum::IDLE, true, 0);  // Persistent, no timeout (initial state)

    // Set motion detection start time (current time + startup delay)
    motionDetectionStartTime = millis() + motionDetectionStartupDelay;

    // Create sensor reading task on Core 1 (separate from rendering for better performance)
    // This prevents slow I2C sensors (like VL6180X) from blocking the render task
    xTaskCreatePinnedToCore(readSensorTask, "SensorTask", 4096, this, 2, &sensorTaskHandle, 1);

    // Create rendering task on Core 0 (dedicated core for smooth rendering)
    // Priority 1 - lower priority since it has the whole core to itself
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

        // Read accelerometer data (safe even if sensor failed to initialize)
        sensors_event_t* event = ctrl->accelerometer.getSensorEvent();

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

        // Read proximity sensor (safe even if sensor failed to initialize)
        if (ctrl->proximitySensor != nullptr) {
            ctrl->proximitySensor->read(&ctrl->sensorBuffer[writeBuffer].proximity);
        } else {
            ctrl->sensorBuffer[writeBuffer].proximity = 0;  // No sensor available
        }

        // Atomic swap - readers now use the new buffer
        ctrl->activeBuffer = writeBuffer;

        // Store previous values for idle detection
        ctrl->prevSensorData = ctrl->sensorBuffer[1 - writeBuffer];

        // Only run motion detection if accelerometer initialized successfully
        if (ctrl->accelerometerInitialized) {
            // Check idle/sleep state
            ctrl->checkIdleAndSleep(ctrl, millis());

            // Check motion detection features (shake, tilt, bounce, spin, petting)
            ctrl->checkMotionFeatures(ctrl);
        }

        // Update facial states with new sensor data
        ctrl->mouthState.setSensorData(ctrl->sensorBuffer[ctrl->activeBuffer]);
        ctrl->eyeState.setSensorData(ctrl->sensorBuffer[ctrl->activeBuffer]);
    }
}

const SensorData& KMMXController::getSensorData() const {
    return sensorBuffer[activeBuffer];
}
