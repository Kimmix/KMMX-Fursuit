#pragma once

#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

/**
 * @brief LIS3DH Accelerometer implementation for KimmixControllerV2
 *
 * This class wraps the Adafruit LIS3DH library.
 *
 * Hardware Details:
 * - I2C Address: 0x18
 * - Data Rate: 50Hz (matches sensor polling rate)
 * - Range: ±2G (sufficient for motion detection)
 */
class LIS3DH {
   private:
    sensors_event_t event;
    Adafruit_LIS3DH lis;
    bool sensorInitialized = false;

   public:
    LIS3DH();
    bool setUp();
    sensors_event_t* getSensorEvent();
};
