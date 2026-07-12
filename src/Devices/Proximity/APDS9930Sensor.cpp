#include "APDS9930Sensor.h"
#include "Utils/Utils.h"  // Use shared utility functions

/**
 * Normalize proximity data to 0-1023 range.
 *
 * Applies minimum and maximum thresholds, auto-calibrates the maximum value,
 * and maps the value to a 0-1023 range. This helps filter out noise and
 * normalize readings across different lighting conditions.
 *
 * @param value Reference to the proximity value to be normalized
 */
void APDS9930Sensor::normalizeProximity(uint16_t &value) {
    // Filter out extremely high values (likely errors)
    if (value > PROX_MAX_VALUE) {
        value = 0;
        return;
    }

    const bool saturated = value >= PROX_SENSOR_MAX;
    value = value > PROX_OFFSET ? value - PROX_OFFSET : 0;

    // Auto-calibrate maximum value for dynamic range adjustment
    if (value > proximity_max) {
        proximity_max = value;
    }

    // Filter out noise below minimum threshold
    if (value < PROX_MIN_VALUE) {
        value = 0;
        return;
    }

    // Map to 0-1023 range using optimized fastMap from Utils
    const uint16_t normalized = fastMap<uint16_t>(value, 0, proximity_max, 0, 1023);
    value = saturated ? 1023 : (normalized < 1023 ? normalized : 1022);
}

/**
 * Initializes the APDS9930 sensor.
 *
 * Sets up the sensor with optimized gain settings and enables the
 * proximity sensor. Configures LED current for better range.
 *
 * @return true if initialization was successful
 */
bool APDS9930Sensor::setup() {
    // Initialize buffer for sensor readings (via base class)
    initializeBuffer();

    // Initialize APDS-specific state
    proximity_max = 1;  // Reset auto-calibration

    // Initialize sensor
    sensorInitialized = apds.init();

    if (sensorInitialized) {
        // Configure sensor settings
        apds.setProximityGain(PGAIN_4X);          // Set optimal gain for proximity sensor
        apds.setLEDDrive(LED_DRIVE_100MA);        // Set LED current to 100mA for better range
        apds.enableProximitySensor(false);        // Enable proximity sensor
    }

    return sensorInitialized;
}

/**
 * Reads and processes data from the sensor.
 *
 * Gets the current proximity reading, applies normalization, and uses median filter
 * for noise rejection.
 *
 * @param proximityData Pointer to store the processed proximity data
 */
void APDS9930Sensor::read(uint16_t *proximityData) {
    if (!sensorInitialized) {
        // Return safe default value (no object nearby) if sensor not initialized
        *proximityData = 0;
        return;
    }

    if (apds.readProximity(*proximityData)) {
        normalizeProximity(*proximityData);
        addProximityToBuffer(*proximityData);
        cachedProximity = medianFilter();
        *proximityData = cachedProximity;
    } else {
        *proximityData = medianFilter();
    }
}
