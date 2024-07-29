#pragma once

#include "config.h"

class HornLED {
   private:
    // Variable to store the brightness level of the horn LED
    unsigned short brightness = hornInitBrightness;

   public:
    HornLED();

    // Method to set the brightness of the horn LED
    // @param value: The desired brightness level (0-100)
    void setBrightness(unsigned short value);

    // Method to get the current brightness of the horn LED
    // @return: The current brightness level
    unsigned short getBrightness() const;
};
