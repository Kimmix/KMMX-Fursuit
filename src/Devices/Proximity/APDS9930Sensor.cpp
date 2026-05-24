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
 * Normalize proximity data to 0-1023 range.
 *
 * Applies minimum and maximum thresholds, auto-calibrates the maximum value,
 * and maps the value to a 0-1023 range. This helps filter out noise and
 * normalize readings across different lighting conditions.
 *
 * @param value Reference to the proximity value to be normalized
 */
void APDS9930Sensor::normalizeProximity(uint16_t &value) {
    // Apply offset correction
    value -= PROX_OFFSET;

    // Filter out extremely high values (likely errors)
    if (value > PROX_MAX_VALUE) {
        value = 0;
        return;
    }

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
    value = fastMap<uint16_t>(value, 0, proximity_max, 0, 1023);
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
    // Initialize buffer for sensor readings (via base class)
    initializeBuffer();

    // Initialize APDS-specific state
    ambientLightReadCounter = 0;
    lastAmbientLight = 0.0f;
    proximity_max = 1;  // Reset auto-calibration

    // Initialize sensor
    sensorInitialized = apds.init();

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
 * Gets the current proximity reading, applies normalization, and uses median filter
 * for noise rejection. Ambient light is read periodically to reduce overhead.
 *
 * PERFORMANCE NOTE: Unlike VL6180X, APDS9930 is fast enough that throttling is
 * typically not needed (READ_SKIP_COUNT = 1). However, the throttling mechanism
 * is available for consistency and can be adjusted if needed.
 *
 * @param proximityData Pointer to store the processed proximity data
 */
void APDS9930Sensor::read(uint16_t *proximityData) {
    if (!sensorInitialized) {
        // Return safe default value (no object nearby) if sensor not initialized
        *proximityData = 0;
        return;
    }

    // Throttle sensor reads if configured (typically disabled for APDS9930)
    readSkipCounter++;
    if (readSkipCounter < READ_SKIP_COUNT) {
        // Return cached value
        *proximityData = cachedProximity;
        return;
    }
    readSkipCounter = 0;

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
            // Normalize proximity data to 0-1023 range
            normalizeProximity(*proximityData);

            // Add to buffer and apply median filter (via base class)
            addProximityToBuffer(*proximityData);
            uint16_t filtered = medianFilter();

            // Cache result for throttled reads
            cachedProximity = filtered;
            *proximityData = filtered;
        } else {
            // Error reading - use last valid reading from buffer
            *proximityData = medianFilter();
        }
    } else {
        // Fast proximity-only read
        if (apds.readProximity(*proximityData)) {
            // Normalize proximity data to 0-1023 range
            normalizeProximity(*proximityData);

            // Add to buffer and apply median filter (via base class)
            addProximityToBuffer(*proximityData);
            uint16_t filtered = medianFilter();

            // Cache result for throttled reads
            cachedProximity = filtered;
            *proximityData = filtered;
        } else {
            // Error reading - use last valid reading from buffer
            *proximityData = medianFilter();
        }
    }
}
