#pragma once

#include <Adafruit_NeoPixel.h>
#include "config.h"

class CheekPanel {
   public:
    CheekPanel(int pin, int numLeds = argbCount);
    void init();
    void update();

   private:
    int pin;
    int numLeds;
    Adafruit_NeoPixel strip;
    unsigned long previousMillis;
    const long interval;
    int shimmerIncrement;

    int lerpColorComponent(int startValue, int endValue, float t);
    int gammaCorrection(int value);

    static const int gammaTable[256];
};