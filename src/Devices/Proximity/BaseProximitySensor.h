#pragma once

#include <Arduino.h>
#include "IProximitySensor.h"

/**
 * Base class for proximity sensors with common buffering and filtering logic.
 * 
 * This abstract class provides shared functionality for proximity sensors including:
 * - Circular buffer management for sensor readings
 * - Median filtering for noise rejection
 * - Throttling mechanism for performance optimization
 * - Debug output timing control
 * 
 * Derived classes must implement sensor-specific normalization logic.
 */
class BaseProximitySensor : public IProximitySensor {
protected:
    // Sensor state
    bool sensorInitialized = false;
    bool debugEnabled = false;

    // Filtering parameters - using consistent values across all sensors
    static constexpr int AVERAGING_SAMPLES = 5;    // Number of samples for median filter (must be odd)
    uint16_t proximityBuffer[AVERAGING_SAMPLES];   // Circular buffer for proximity readings
    int bufferIndex = 0;                           // Current position in the circular buffer
    bool bufferFilled = false;                     // Whether the buffer has been filled at least once

    // Throttling mechanism for performance optimization
    uint8_t readSkipCounter = 0;                   // Counter for skipping sensor reads
    uint16_t cachedProximity = 0;                  // Last valid proximity reading (used when throttling)

    // Debug timing
    static constexpr unsigned long DEBUG_INTERVAL = 100; // Debug output interval in milliseconds
    unsigned long lastDebugTime = 0;

    /**
     * Add a proximity reading to the circular buffer.
     * 
     * @param value The proximity value to add to the buffer
     */
    void addProximityToBuffer(uint16_t value);

    /**
     * Apply median filter to proximity readings for noise rejection.
     * 
     * Median filter is more effective than averaging for removing outliers/spikes.
     * Uses the optimized generic medianFilter from Utils.
     * 
     * @return The median proximity value from the buffer
     */
    uint16_t medianFilter();

    /**
     * Initialize the proximity buffer with zeros.
     */
    void initializeBuffer();

    /**
     * Check if enough time has passed for debug output.
     * 
     * @return true if debug output should be printed
     */
    bool shouldPrintDebug();

public:
    virtual ~BaseProximitySensor() = default;

    /**
     * Check if the sensor is initialized.
     * 
     * @return true if the sensor is initialized
     */
    bool isInitialized() const override { return sensorInitialized; }
};
