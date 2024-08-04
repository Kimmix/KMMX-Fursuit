#pragma once

#include <Adafruit_NeoPixel.h>
#include "GammaTable.h"
#include "config.h"

class CheekPanel {
   public:
    CheekPanel(uint16_t numLeds, uint8_t pin);
    void configure(uint32_t backgroundColor, uint32_t fadeColor, uint16_t fadeInterval, uint16_t positionChangeDelay);
    void init();
    void update();

   private:
    Adafruit_NeoPixel strip;
    uint32_t backgroundColor;
    uint32_t fadeColor;
    uint16_t fadeInterval;
    uint16_t positionChangeDelay;
    unsigned long previousMillis;
    unsigned long positionChangeMillis;
    int fadeStep = 0;
    int fadeDirection = 1;
    int randomPosition = 0;
    const int fadeSteps = 50;

    void setBackgroundColor(uint32_t color);
    uint32_t applyGammaCorrection(uint32_t color);
};