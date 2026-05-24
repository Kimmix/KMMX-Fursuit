#pragma once

#include <APDS9930.h>
#include "config.h"
#include "BaseProximitySensor.h"

/**
 * APDS9930 Proximity and Ambient Light sensor driver.
 *
 * This class implements the BaseProximitySensor interface for the APDS9930 sensor.
 * The APDS9930 provides proximity detection using an IR LED and photodiode, along with
 * ambient light sensing. The sensor output is normalized to a 0-1023 range for compatibility
 * with the existing boop detection logic.
 *
 * Key features:
 * - IR-based proximity detection with adjustable gain and LED current
 * - Ambient light sensing (updated periodically to reduce overhead)
 * - Median filtering for noise rejection (via base class)
 * - Automatic normalization to 0-1023 range
 * - I2C communication (default address 0x39)
 */
class APDS9930Sensor : public BaseProximitySensor {
   private:
    APDS9930 apds = APDS9930();
    int proximity_max = 1;  // Dynamic maximum value for auto-calibration

    // Constants for sensor data processing
    static constexpr int PROX_MIN_VALUE = 50;     // Minimum valid proximity value
    static constexpr int PROX_MAX_VALUE = 50000;  // Maximum valid proximity value
    static constexpr int PROX_OFFSET = 200;       // Offset value to adjust proximity readings

    // Performance optimization: optional throttling (disabled by default for APDS9930)
    // The APDS9930 is faster than VL6180X, so throttling is optional
    static constexpr uint8_t READ_SKIP_COUNT = 1;  // Read every call (no throttling)

    // Ambient light reading optimization
    uint32_t ambientLightReadCounter = 0;         // Counter for periodic ambient light reads
    float lastAmbientLight = 0.0f;                // Last ambient light reading

    /**
     * Read proximity and ambient light data from the sensor.
     *
     * @param proximityData Reference to store the proximity data
     * @param ambientLight Reference to store the ambient light data in lux
     * @return true if both proximity and ambient light readings were successful
     */
    bool readAPDSSensor(uint16_t &proximityData, float &ambientLight);

    /**
     * Normalize proximity data to 0-1023 range.
     *
     * Applies minimum and maximum thresholds, auto-calibrates the maximum value,
     * and maps the value to a 0-1023 range. This helps filter out noise and
     * normalize readings across different lighting conditions.
     *
     * @param value Reference to the proximity value to be normalized
     */
    void normalizeProximity(uint16_t &value);

   public:
    /**
     * Initialize the APDS9930 sensor.
     *
     * @return true if initialization was successful
     */
    bool setup() override;

    /**
     * Read and process data from the sensor.
     *
     * @param proximityData Pointer to store the processed proximity data
     */
    void read(uint16_t *proximityData) override;
};
