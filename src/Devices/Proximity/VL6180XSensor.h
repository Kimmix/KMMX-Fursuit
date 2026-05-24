#pragma once

#include <VL6180X.h>
#include "config.h"
#include "IProximitySensor.h"

/**
 * VL6180X Time-of-Flight (ToF) proximity sensor driver.
 *
 * This class implements the IProximitySensor interface for the VL6180X ToF sensor.
 * The VL6180X provides accurate distance measurements up to ~200mm using time-of-flight
 * technology. The sensor output is normalized to a 0-1023 range for compatibility with
 * the existing boop detection logic.
 *
 * Key features:
 * - Time-of-Flight (ToF) technology for accurate ranging
 * - Typical range: 0-200mm (with configurable scaling)
 * - Median filtering for noise rejection
 * - Automatic normalization to 0-1023 range
 * - I2C communication (default address 0x29)
 */
class VL6180XSensor : public IProximitySensor {
   private:
    VL6180X sensor;
    bool sensorInitialized = false;
    bool debugEnabled = false;  // Flag to control debug output

    // Constants for sensor data processing
    static constexpr uint8_t VL6180X_MAX_RANGE = 200;      // Maximum reliable range in mm
    static constexpr uint8_t VL6180X_MIN_RANGE = 50;        // Minimum valid range in mm (ignore very close readings)
    static constexpr uint8_t VL6180X_ERROR_THRESHOLD = 250; // Values above this are errors

    // Performance optimization: skip sensor reads to reduce I2C overhead
    // The VL6180X is slow (~30ms per read), so we only read every N calls
    static constexpr uint8_t READ_SKIP_COUNT = 3;  // Read every 3rd call (60ms @ 20ms sensor task)
    uint8_t readSkipCounter = 0;

    // Filtering parameters (matching APDS9930 for consistency)
    static constexpr int AVERAGING_SAMPLES = 5;    // Number of samples to average (must be odd for median)
    uint16_t distanceBuffer[AVERAGING_SAMPLES];    // Buffer for distance readings
    int bufferIndex = 0;                           // Current position in the buffer
    bool bufferFilled = false;                     // Whether the buffer has been filled once

    /**
     * Normalize distance reading to 0-1023 range.
     *
     * The VL6180X returns distance in millimeters. We need to:
     * 1. Filter out invalid readings (errors, out of range)
     * 2. Invert the reading (closer = higher value)
     * 3. Map to 0-1023 range for boop compatibility
     *
     * @param distanceMm The raw distance reading in millimeters
     * @return Normalized proximity value (0 = far/no object, 1023 = very close)
     */
    uint16_t normalizeDistance(uint8_t distanceMm);

    /**
     * Add a distance reading to the averaging buffer.
     *
     * @param value The distance value to add to the buffer
     */
    void addDistanceToBuffer(uint16_t value);

    /**
     * Apply median filter to distance readings for better noise rejection.
     *
     * Median filter is more effective than averaging for removing outliers/spikes.
     *
     * @return The median distance value
     */
    uint16_t medianFilter();

   public:
    /**
     * Initialize the VL6180X sensor.
     *
     * @return true if initialization was successful
     */
    bool setup() override;

    /**
     * Read and process data from the sensor.
     *
     * Gets the current distance reading, applies filtering, and normalizes
     * to 0-1023 range for compatibility with boop detection logic.
     *
     * @param proximityData Pointer to store the processed proximity data
     */
    void read(uint16_t *proximityData) override;

    /**
     * Check if the sensor is initialized.
     *
     * @return true if the sensor is initialized
     */
    bool isInitialized() const override { return sensorInitialized; }
};
