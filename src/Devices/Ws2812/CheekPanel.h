#pragma once

#include <Adafruit_NeoPixel.h>
#include "config.h"

class CheekPanel {
   public:
    CheekPanel(uint16_t pin, uint16_t numLeds = argbCount);
    void init();
    void update();

   private:
    Adafruit_NeoPixel strip;
    uint16_t numLeds;
    uint16_t hue1;
    uint16_t hue2;
    float offset;
    unsigned long lastUpdateTime;

    uint32_t colorFromHue(uint16_t hue);
};
