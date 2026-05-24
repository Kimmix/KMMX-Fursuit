#include "VL6180XSensor.h"
#include "Utils/Utils.h"  // Use shared utility functions

/**
 * Normalize distance reading to 0-1023 range.
 *
 * The VL6180X returns distance in millimeters (0-200mm typical range).
 * We need to invert this (closer = higher value) and map to 0-1023.
 */
uint16_t VL6180XSensor::normalizeDistance(uint8_t distanceMm) {
    // Check for error readings (VL6180X returns 255 for errors)
    if (distanceMm >= VL6180X_ERROR_THRESHOLD) {
        return 0;  // No valid object detected
    }

    // Filter out readings that are too close (likely sensor noise or object touching sensor)
    if (distanceMm < VL6180X_MIN_RANGE) {
        // Very close readings map to maximum proximity (1023)
        return 1023;
    }

    // Filter out readings beyond max range
    if (distanceMm > VL6180X_MAX_RANGE) {
        return 0;  // Too far, no object detected
    }

    // Invert the distance (closer = higher value) and map to 0-1023
    // When distance is VL6180X_MIN_RANGE (50mm), output should be 1023
    // When distance is VL6180X_MAX_RANGE (200mm), output should be 0
    uint16_t invertedDistance = VL6180X_MAX_RANGE - distanceMm;

    // Map from [0, VL6180X_MAX_RANGE - VL6180X_MIN_RANGE] to [0, 1023]
    // Use optimized fastMap from Utils
    uint16_t normalized = fastMap<uint16_t>(invertedDistance, 0, VL6180X_MAX_RANGE - VL6180X_MIN_RANGE, 0, 1023);

    return normalized;
}

/**
 * Initialize the VL6180X sensor.
 */
bool VL6180XSensor::setup() {
    // Initialize buffer for sensor readings (via base class)
    initializeBuffer();

    // Initialize sensor
    sensor.init();
    sensor.configureDefault();

    // Set timeout for ranging measurements (shorter for faster failure detection)
    sensor.setTimeout(50);  // 50ms timeout (much shorter than default)

    // Reduce max convergence time for faster measurements
    sensor.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);  // 30ms max (default is 49ms)

    // Test if sensor is responding by attempting a single reading
    uint8_t testReading = sensor.readRangeSingleMillimeters();

    // Check if we got a timeout (sensor not present)
    if (sensor.timeoutOccurred()) {
        sensorInitialized = false;
        if (debugEnabled) {
            Serial.println(F("VL6180X initialization failed - timeout"));
        }
        return false;
    }

    sensorInitialized = true;

    if (debugEnabled) {
        Serial.println(F("VL6180X initialization complete (throttled mode)"));
        Serial.printf("Initial range reading: %d mm\n", testReading);
        Serial.printf("Read frequency: every %d calls\n", READ_SKIP_COUNT);
    }

    return sensorInitialized;
}

/**
 * Read and process data from the sensor.
 *
 * PERFORMANCE OPTIMIZATION: Skip sensor reads to reduce I2C overhead.
 * The VL6180X is inherently slow (~30ms per read). Instead of trying to make it faster,
 * we read it less frequently. For boop detection, 60ms update rate is still very responsive.
 */
void VL6180XSensor::read(uint16_t *proximityData) {
    if (!sensorInitialized) {
        // Return safe default value (no object nearby) if sensor not initialized
        *proximityData = 0;
        return;
    }

    // Use static variable to cache last valid distance for debugging
    static uint8_t lastValidDistance = 255;

    // Throttle sensor reads - only read every READ_SKIP_COUNT calls
    readSkipCounter++;
    if (readSkipCounter < READ_SKIP_COUNT) {
        // Return cached value (no sensor read = no blocking!)
        *proximityData = cachedProximity;
        return;
    }

    // Time to do an actual sensor read
    readSkipCounter = 0;

    // Read distance from sensor (blocking, but only happens every 60ms)
    uint8_t distance = sensor.readRangeSingleMillimeters();

    if (sensor.timeoutOccurred()) {
        if (debugEnabled && shouldPrintDebug()) {
            Serial.println(F("VL6180X timeout, using cached value"));
        }
        // Use last valid reading on timeout
        *proximityData = cachedProximity;
        return;
    }

    // Valid reading received
    lastValidDistance = distance;

    // Normalize distance to 0-1023 range
    uint16_t normalized = normalizeDistance(distance);

    // Add to buffer and apply median filter (via base class)
    addProximityToBuffer(normalized);
    uint16_t filtered = medianFilter();

    // Cache result for throttled reads
    cachedProximity = filtered;
    *proximityData = filtered;

    // Print debug information if debug is enabled and interval has passed
    if (debugEnabled && shouldPrintDebug()) {
        Serial.print(F("VL6180X - Distance: "));
        Serial.print(distance);
        Serial.print(F(" mm, Normalized: "));
        Serial.println(*proximityData);
    }
}
