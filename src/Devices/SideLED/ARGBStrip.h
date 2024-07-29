#pragma once

#include <FastLED.h>
#include "config.h"

class ARGBStrip {
   public:
    ARGBStrip();
    void animate();

   private:
    CRGB leds[24];
    unsigned long prevTime = 0;

    void FillLEDsFromPaletteColors(uint8_t colorIndex);
    CRGBPalette16 lavaLampPalette();
};
