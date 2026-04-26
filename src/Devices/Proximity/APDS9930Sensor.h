#pragma once

#include <APDS9930.h>
#include "config.h"
#include "esp_dsp.h"  // ESP-DSP for hardware-accelerated operations

class APDS9930Sensor {
   private:
    APDS9930 apds = APDS9930();
    bool sensorInitialized = false;
    int proximity_max = 1;
    bool debugEnabled = false;  // Flag to control debug output

    // Constants for sensor data processing
    static constexpr int PROX_MIN_VALUE = 50;     // Minimum valid proximity value
    static constexpr int PROX_MAX_VALUE = 50000;  // Maximum valid proximity value
    static constexpr int PROX_OFFSET = 200;       // Offset value to adjust proximity readings

    // Filtering parameters
    static constexpr int AVERAGING_SAMPLES = 5;    // Number of samples to average (must be odd for median)
    uint16_t proximityBuffer[AVERAGING_SAMPLES];   // Buffer for proximity readings
    int bufferIndex = 0;                          // Current position in the buffer
    bool bufferFilled = false;                    // Whether the buffer has been filled once
    uint32_t ambientLightReadCounter = 0;         // Counter for periodic ambient light reads
    float lastAmbientLight = 0.0f;                // Last ambient light reading

    bool readAPDSSensor(uint16_t &proximityData, float &ambientLight);
    void filterProx(uint16_t &value);
    uint16_t medianFilter();
    uint16_t averageProximity();
    void addProximityToBuffer(uint16_t value);

   public:
    bool setup();
    void read(uint16_t *proximityData);
    bool isInitialized() const { return sensorInitialized; }
};
