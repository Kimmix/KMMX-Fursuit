#include "APDS9930Sensor.h"
#include "Utils/Utils.h"  // Use shared utility functions

/**
 * Reads proximity and ambient light data from the APDS9930 sensor.
 *
 * @param proximityData Reference to store the proximity data
 * @param ambientLight Reference to store the ambient light data in lux
 * @return true if both proximity and ambient light readings were successful
 */
bool APDS9930Sensor::readAPDSSensor(uint16_t &proximityData, float &ambientLight) {
    if (apds.readProximity(proximityData) && apds.readAmbientLightLux(ambientLight)) {
        return true;
    }
    return false;
}

/**
 * Filters and normalizes proximity data.
 *
 * Applies minimum and maximum thresholds, and maps the value to a 0-1023 range.
 * This helps filter out noise and normalize readings across different lighting conditions.
 *
 * @param value Reference to the proximity value to be filtered
 */
void APDS9930Sensor::filterProx(uint16_t &value) {
    value -= PROX_OFFSET;
    if (value > PROX_MAX_VALUE) {
        value = 0;
    }
    if (value > proximity_max) {
        proximity_max = value;
    }
    if (value < PROX_MIN_VALUE) {
        value = 0;
    }
    // Use optimized fastMap from Utils instead of Arduino's map()
    value = fastMap<uint16_t>(value, 0, proximity_max, 0, 1023);
}

/**
 * Adds a proximity reading to the averaging buffer.
 *
 * @param value The proximity value to add to the buffer
 */
void APDS9930Sensor::addProximityToBuffer(uint16_t value) {
    proximityBuffer[bufferIndex] = value;
    bufferIndex = (bufferIndex + 1) % AVERAGING_SAMPLES;

    if (bufferIndex == 0) {
        bufferFilled = true;
    }
}

/**
 * Applies median filter to proximity readings for better noise rejection.
 * Uses the optimized generic medianFilter from Utils.
 *
 * Median filter is more effective than averaging for removing outliers/spikes.
 *
 * @return The median proximity value
 */
uint16_t APDS9930Sensor::medianFilter() {
    if (!bufferFilled && bufferIndex == 0) {
        return 0; // No data yet
    }

    int count = bufferFilled ? AVERAGING_SAMPLES : bufferIndex;

    // Use optimized generic median filter from Utils
    return ::medianFilter<uint16_t, AVERAGING_SAMPLES>(proximityBuffer, count);
}

/**
 * Calculates the average of recent proximity readings using ESP-DSP power/sum function.
 * Kept for compatibility, but median filter is preferred.
 *
 * Uses hardware-accelerated sum calculation on ESP32-S3 via dot product.
 * sum = dotprod(buffer, ones_vector) which uses SIMD instructions.
 *
 * @return The average proximity value
 */
uint16_t APDS9930Sensor::averageProximity() {
    if (!bufferFilled && bufferIndex == 0) {
        return 0; // No data yet
    }

    int count = bufferFilled ? AVERAGING_SAMPLES : bufferIndex;

    // Convert to float array for ESP-DSP processing
    float floatBuffer[AVERAGING_SAMPLES];
    float onesVector[AVERAGING_SAMPLES];

    for (int i = 0; i < count; i++) {
        floatBuffer[i] = static_cast<float>(proximityBuffer[i]);
        onesVector[i] = 1.0f;
    }

    // Use ESP-DSP dot product to calculate sum (hardware-accelerated)
    // sum = dotprod(buffer, ones) where ones = [1,1,1,1,1]
    float sum;
    dsps_dotprod_f32(floatBuffer, onesVector, &sum, count);

    return static_cast<uint16_t>(sum / count);
}

/**
 * Initializes the APDS9930 sensor.
 *
 * Sets up the sensor with optimized gain settings and enables both
 * the proximity sensor and light sensor. Configures LED current and
 * timing for better accuracy.
 *
 * @return true if initialization was successful
 */
bool APDS9930Sensor::setup() {
    // Initialize buffer for sensor readings
    for (int i = 0; i < AVERAGING_SAMPLES; i++) {
        proximityBuffer[i] = 0;
    }
    bufferIndex = 0;
    bufferFilled = false;
    ambientLightReadCounter = 0;
    lastAmbientLight = 0.0f;

    // Initialize sensor
    sensorInitialized = apds.init();

    if (sensorInitialized) {
        // Configure sensor settings
        apds.setProximityGain(PGAIN_4X);          // Set optimal gain for proximity sensor
        apds.setLEDDrive(LED_DRIVE_100MA);        // Set LED current to 100mA for better range
        apds.enableProximitySensor(false);        // Enable proximity sensor
        apds.enableLightSensor(false);            // Enable light sensor

        if (debugEnabled) {
            Serial.println(F("APDS9930 initialization complete"));
        }
    } else if (debugEnabled) {
        Serial.println(F("APDS9930 initialization failed!"));
    }

    return sensorInitialized;
}

/**
 * Reads and processes data from the sensor.
 *
 * Gets the current proximity reading, applies filtering, and uses median filter
 * for noise rejection. Ambient light is read periodically to reduce overhead.
 *
 * @param proximityData Pointer to store the processed proximity data
 * @return void
 */
void APDS9930Sensor::read(uint16_t *proximityData) {
    if (!sensorInitialized) {
        // Return safe default value (no object nearby) if sensor not initialized
        *proximityData = 0;
        return;
    }

    // Use static variable to track debug timing
    static unsigned long lastDebugTime = 0;
    const unsigned long debugInterval = 100; // Debug output every 100ms

    // Read ambient light periodically (every 50 samples ~1 second at 50Hz)
    // This reduces overhead and allows faster proximity reads
    ambientLightReadCounter++;
    bool readLight = (ambientLightReadCounter >= 50);
    if (readLight) {
        ambientLightReadCounter = 0;
    }

    if (readLight) {
        // Full read with ambient light
        if (readAPDSSensor(*proximityData, lastAmbientLight)) {
            // Apply proximity filtering to normalize readings
            filterProx(*proximityData);

            // Add to averaging buffer
            addProximityToBuffer(*proximityData);

            // Use median filter for better noise rejection
            *proximityData = medianFilter();
        } else {
            // Error reading - use last valid reading
            if (debugEnabled && (millis() - lastDebugTime > debugInterval)) {
                lastDebugTime = millis();
                Serial.println(F("APDS9930 error reading sensor"));
            }
            *proximityData = medianFilter();
        }
    } else {
        // Fast proximity-only read
        if (apds.readProximity(*proximityData)) {
            // Apply proximity filtering to normalize readings
            filterProx(*proximityData);

            // Add to averaging buffer
            addProximityToBuffer(*proximityData);

            // Use median filter for better noise rejection
            *proximityData = medianFilter();
        } else {
            // Error reading - use last valid reading
            *proximityData = medianFilter();
        }
    }

    // Print debug information if debug is enabled and interval has passed
    if (debugEnabled && (millis() - lastDebugTime > debugInterval)) {
        lastDebugTime = millis();
        Serial.print(F("APDS9930 - Light: "));
        Serial.print(lastAmbientLight);
        Serial.print(F(" lux, Proximity: "));
        Serial.println(*proximityData);
    }
}
