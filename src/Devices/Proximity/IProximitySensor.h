#pragma once

#include <Arduino.h>

/**
 * Interface for proximity sensors.
 *
 * This interface provides a unified API for different proximity sensor types,
 * allowing for polymorphic use and runtime sensor selection.
 *
 * CRITICAL NORMALIZATION CONTRACT:
 * ================================
 * All proximity sensors MUST normalize their output to the 0-1023 range:
 * - 0 = No object detected, too far, or sensor error
 * - Higher values = Closer proximity
 * - 1023 = Extremely close/touching sensor (triggers ANGRY boop state)
 *
 * Normalization approaches by sensor type:
 *
 * VL6180X (Time-of-Flight):
 * - Measures distance in millimeters (0-200mm typical range)
 * - Inverts distance: closer = higher value
 * - Maps [50mm, 200mm] -> [1023, 0]
 * - Values < 50mm are clamped to 1023 (too close)
 * - Uses median filtering on normalized values for noise reduction
 *
 * APDS9930 (IR Proximity):
 * - Reads raw proximity values (0-50000 typical range)
 * - Applies offset correction and auto-calibration
 * - Maps [PROX_MIN_VALUE, proximity_max] -> [0, 1023]
 * - Auto-adjusts to lighting conditions via dynamic max tracking
 * - Uses median filtering on normalized values for noise reduction
 *
 * Both implementations:
 * - Use median filter (5 samples) for outlier rejection
 * - Return cached values when sensors timeout/error
 * - Ensure thread-safe operation for task-based systems
 *
 * Boop detection thresholds (from config.h):
 * - boopMinThreshold: 100 (object enters range)
 * - boopMaxThreshold: 900 (full boop trigger)
 * - 1023: Angry state (object too close)
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
