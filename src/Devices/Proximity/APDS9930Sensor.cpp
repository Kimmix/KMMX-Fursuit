#include "APDS9930Sensor.h"

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
    value = map(value, 0, proximity_max, 0, 1023);
}

/**
 * Compensates for high ambient light conditions.
 *
 * When ambient light is above the sunlight threshold, it can interfere with proximity
 * readings, so we zero out the proximity value to avoid false readings.
 *
 * @param proximity Reference to the proximity value
 * @param ambientLight The ambient light level in lux
 */
void APDS9930Sensor::conquerTheSun(uint16_t &proximity, float &ambientLight) {
    if (ambientLight > SUNLIGHT_THRESHOLD) {
        proximity = 0;
    }
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
 * Calculates the average of recent proximity readings.
 *
 * @return The average proximity value
 */
uint16_t APDS9930Sensor::averageProximity() {
    if (!bufferFilled && bufferIndex == 0) {
        return 0; // No data yet
    }

    uint32_t sum = 0;
    int count = bufferFilled ? AVERAGING_SAMPLES : bufferIndex;

    for (int i = 0; i < count; i++) {
        sum += proximityBuffer[i];
    }

    return (count > 0) ? static_cast<uint16_t>(sum / count) : 0;
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

    // Initialize sensor
    sensorInitialized = apds.init();

    if (debugEnabled && sensorInitialized) {
        Serial.println(F("APDS-9930 initialization complete"));
    } else if (debugEnabled) {
        Serial.println(F("Something went wrong during APDS-9930 init!"));
    }

    if (sensorInitialized) {
        // Configure sensor settings
        apds.setProximityGain(PGAIN_4X);          // Set optimal gain for proximity sensor
        apds.setLEDDrive(LED_DRIVE_100MA);        // Set LED current to 100mA for better range
        apds.enableProximitySensor(false);        // Enable proximity sensor
        apds.enableLightSensor(false);            // Enable light sensor
    }

    return sensorInitialized;
}

/**
 * Reads and processes data from the sensor.
 *
 * Gets the current proximity and ambient light readings, applies
 * filtering to account for environmental conditions, and performs
 * averaging for more stable readings.
 *
 * @param proximityData Pointer to store the processed proximity data
 * @return void
 */
void APDS9930Sensor::read(uint16_t *proximityData) {
    if (!sensorInitialized) {
        return;
    }
    float ambientLight;

    // Use static variable to track debug timing
    static unsigned long lastDebugTime = 0;
    const unsigned long debugInterval = 100; // Debug output every 100ms

    if (readAPDSSensor(*proximityData, ambientLight)) {
        conquerTheSun(*proximityData, ambientLight);

        // Apply proximity filtering to normalize readings
        filterProx(*proximityData);

        // Add to averaging buffer
        addProximityToBuffer(*proximityData);

        // Get averaged proximity value for more stable readings
        *proximityData = averageProximity();

        // Print debug information if debug is enabled and interval has passed
        if (debugEnabled && (millis() - lastDebugTime > debugInterval)) {
            lastDebugTime = millis();
            Serial.print(F("AmbientLight: "));
            Serial.print(ambientLight);
            Serial.print(F(" lux, Raw Proximity: "));
            Serial.print(*proximityData);
            Serial.print(F(", Filtered Proximity: "));
            Serial.println(averageProximity());
        }
    } else {
        Serial.println(F("Error reading sensor data."));
    }
}
