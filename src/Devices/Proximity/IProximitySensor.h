#pragma once

#include <Arduino.h>

/**
 * Interface for proximity sensors.
 *
 * Provides a unified API for different proximity sensor types, enabling
 * polymorphic use and runtime sensor selection.
 *
 * NORMALIZATION CONTRACT:
 * ======================
 * All sensors MUST output values in the 0-1023 range:
 * - 0 = No object detected / too far / sensor error
 * - Higher values = Closer proximity
 * - 1023 = Extremely close/touching (triggers ANGRY boop state)
 *
 * VL6180X (Time-of-Flight):
 * - Measures distance in mm, inverts to proximity (closer = higher)
 * - Maps [50mm, 200mm] -> [1023, 0]; values <50mm clamp to 1023
 *
 * APDS9930 (IR Proximity):
 * - Applies offset correction and auto-calibration to raw values
 * - Dynamically adjusts to lighting conditions
 *
 * Both implementations use 5-sample median filtering and return cached
 * values on timeout/error.
 *
 * Boop thresholds (config.h):
 * - boopMinThreshold: Object enters boop range
 * - boopMaxThreshold: Full boop trigger point
 * - 1023: Triggers ANGRY state
 */
class IProximitySensor {
   public:
    virtual ~IProximitySensor() = default;

    /**
     * Initialize the proximity sensor.
     *
     * @return true if initialization was successful, false otherwise
     */
    virtual bool setup() = 0;

    /**
     * Read proximity data from the sensor.
     *
     * This method should handle all filtering, normalization, and processing
     * to provide a consistent 0-1023 range output regardless of the underlying
     * sensor hardware. A value of 0 indicates no object detected, while higher
     * values indicate closer proximity.
     *
     * @param proximityData Pointer to store the processed proximity data (0-1023 range)
     */
    virtual void read(uint16_t *proximityData) = 0;

    /**
     * Check if the sensor is initialized and ready for use.
     *
     * @return true if the sensor is initialized, false otherwise
     */
    virtual bool isInitialized() const = 0;
};
