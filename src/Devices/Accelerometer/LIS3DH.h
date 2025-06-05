#pragma once

#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

class LIS3DH {
   private:
    sensors_event_t event;
    Adafruit_LIS3DH lis;

   public:
    LIS3DH();
    void setUp();
    sensors_event_t* getSensorEvent();
    void setDataRate(uint8_t rate);
};
