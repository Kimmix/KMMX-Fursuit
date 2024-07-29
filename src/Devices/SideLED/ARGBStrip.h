#pragma once

#include <FastLED.h>
#include "config.h"

class ARGBStrip {
   public:
    ARGBStrip();
    void animate();

   private:
    const CHSV sideColor1HSV = CHSV(sideColor1Hue, sideColor1Saturation, sideColor1Value);  // Start color
    const CHSV sideColor2HSV = CHSV(sideColor2Hue, sideColor2Saturation, sideColor2Value);  // End color
    CRGB leds[24];
    unsigned long prevTime = 0;

    void FillLEDsFromPaletteColors(uint8_t colorIndex);
    CRGBPalette16 lavaLampPalette();
};
