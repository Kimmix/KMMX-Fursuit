#pragma once

#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

class LIS3DH {
   private:
    sensors_event_t event;
    Adafruit_LIS3DH lis;

    // Motion detection
    float motionThreshold = 0.1;
    bool motionDetected = false;
    unsigned long lastMotionTime = 0;

    // Helper methods
    void updateMotionStatus(sensors_event_t* event);

   public:
    LIS3DH();
    void setUp();
    sensors_event_t* getSensorEvent();

    // Motion detection methods
    bool isMotionDetected();
    void setMotionThreshold(float threshold);
};
