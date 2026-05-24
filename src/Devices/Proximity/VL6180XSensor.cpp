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
    // When distance is VL6180X_MIN_RANGE (5mm), output should be 1023
    // When distance is VL6180X_MAX_RANGE (200mm), output should be 0
    uint16_t invertedDistance = VL6180X_MAX_RANGE - distanceMm;

    // Map from [0, VL6180X_MAX_RANGE - VL6180X_MIN_RANGE] to [0, 1023]
    // Use optimized fastMap from Utils
    uint16_t normalized = fastMap<uint16_t>(invertedDistance, 0, VL6180X_MAX_RANGE - VL6180X_MIN_RANGE, 0, 1023);

    return normalized;
}

/**
 * Add a distance reading to the averaging buffer.
 */
void VL6180XSensor::addDistanceToBuffer(uint16_t value) {
    distanceBuffer[bufferIndex] = value;
    bufferIndex = (bufferIndex + 1) % AVERAGING_SAMPLES;

    if (bufferIndex == 0) {
        bufferFilled = true;
    }
}

/**
 * Apply median filter to distance readings for better noise rejection.
 * Uses the optimized generic medianFilter from Utils.
 */
uint16_t VL6180XSensor::medianFilter() {
    if (!bufferFilled && bufferIndex == 0) {
        return 0; // No data yet
    }

    int count = bufferFilled ? AVERAGING_SAMPLES : bufferIndex;

    // Use optimized generic median filter from Utils
    return ::medianFilter<uint16_t, AVERAGING_SAMPLES>(distanceBuffer, count);
}

/**
 * Initialize the VL6180X sensor.
 */
bool VL6180XSensor::setup() {
    // Initialize buffer for sensor readings
    for (int i = 0; i < AVERAGING_SAMPLES; i++) {
        distanceBuffer[i] = 0;
    }
    bufferIndex = 0;
    bufferFilled = false;
    readSkipCounter = 0;

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

    // Use static variables to track state
    static unsigned long lastDebugTime = 0;
    const unsigned long debugInterval = 100; // Debug output every 100ms
    static uint8_t lastValidDistance = 255;  // Cache last valid distance
    static uint16_t cachedProximity = 0;     // Cache last proximity value

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
        if (debugEnabled && (millis() - lastDebugTime > debugInterval)) {
            lastDebugTime = millis();
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

    // Add to averaging buffer
    addDistanceToBuffer(normalized);

    // Use median filter for better noise rejection
    uint16_t filtered = medianFilter();
    cachedProximity = filtered;
    *proximityData = filtered;

    // Print debug information if debug is enabled and interval has passed
    if (debugEnabled && (millis() - lastDebugTime > debugInterval)) {
        lastDebugTime = millis();
        Serial.print(F("VL6180X - Distance: "));
        Serial.print(distance);
        Serial.print(F(" mm, Normalized: "));
        Serial.println(*proximityData);
    }
}
