#pragma once

#include <Arduino.h>

/**
 * Interface for proximity sensors.
 * 
 * This interface provides a unified API for different proximity sensor types,
 * allowing for polymorphic use and runtime sensor selection. All proximity
 * sensors should normalize their output to a 0-1023 range for consistent
 * "boop" detection across different hardware implementations.
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
