#pragma once

#include <APDS9930.h>
#include "config.h"

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
    static constexpr float SUNLIGHT_THRESHOLD = 1000.0; // Threshold above which we consider it to be direct sunlight

    // Averaging parameters
    static constexpr int AVERAGING_SAMPLES = 5;    // Number of samples to average
    uint16_t proximityBuffer[AVERAGING_SAMPLES];   // Buffer for proximity readings
    int bufferIndex = 0;                          // Current position in the buffer
    bool bufferFilled = false;                    // Whether the buffer has been filled once

    bool readAPDSSensor(uint16_t &proximityData, float &ambientLight);
    void filterProx(uint16_t &value);
    void conquerTheSun(uint16_t &proximity, float &ambientLight);
    uint16_t averageProximity();
    void addProximityToBuffer(uint16_t value);

   public:
    bool setup();
    void read(uint16_t *proximityData);
    bool isInitialized() const { return sensorInitialized; }
};
