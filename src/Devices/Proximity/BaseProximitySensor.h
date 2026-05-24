#pragma once

#include <Arduino.h>
#include "IProximitySensor.h"

/**
 * Base class for proximity sensors with common buffering and filtering.
 *
 * Provides shared functionality:
 * - Circular buffer management for sensor readings
 * - Median filtering (5 samples) for noise rejection
 * - Optional read throttling for performance optimization
 *
 * Derived classes implement sensor-specific normalization to 0-1023 range.
 */
class BaseProximitySensor : public IProximitySensor {
protected:
    // Sensor state
    bool sensorInitialized = false;

    // Filtering (5-sample median filter, consistent across all sensors)
    static constexpr int AVERAGING_SAMPLES = 5;
    uint16_t proximityBuffer[AVERAGING_SAMPLES];
    int bufferIndex = 0;
    bool bufferFilled = false;

    // Throttling for performance optimization
    uint8_t readSkipCounter = 0;
    uint16_t cachedProximity = 0;

    /** Add proximity reading to circular buffer */
    void addProximityToBuffer(uint16_t value);

    /** Apply median filter for noise rejection (uses Utils::medianFilter) */
    uint16_t medianFilter();

    /** Initialize buffer, counters, and timers to zero */
    void initializeBuffer();

public:
    virtual ~BaseProximitySensor() = default;

    /**
     * Check if the sensor is initialized.
     *
     * @return true if the sensor is initialized
     */
    bool isInitialized() const override { return sensorInitialized; }
};
