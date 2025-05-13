#pragma once
#include "config.h"

class HornLED {
   public:
    HornLED();

    // Set brightness with fade speed
    void setBrightness(int value, int speed = 1);

    // Get the current brightness
    int getBrightness() const;

    // Update brightness gradually
    void update();

   private:
    int currentBrightness = 0;  // Current brightness level (0-100)
    int targetBrightness = 0;   // Target brightness level (0-100)
    int fadeSpeed = 1;          // Speed of transition per update
    float pwmValue = 0;           // Cached PWM value for efficiency
    unsigned long previousMillis;
};