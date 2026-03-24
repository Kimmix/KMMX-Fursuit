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
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness() const;
    void setBackgroundColorRGB(uint8_t r, uint8_t g, uint8_t b);
    void setFadeColorRGB(uint8_t r, uint8_t g, uint8_t b);
    uint32_t getBackgroundColor() const;
    uint32_t getFadeColor() const;

   private:
    Adafruit_NeoPixel strip;
    uint32_t backgroundColor;
    uint32_t fadeColor;
    uint16_t fadeInterval;
    uint16_t positionChangeDelay;
    unsigned long previousMillis;
    unsigned long positionChangeMillis;
    uint8_t fadeStep = 0;
    int8_t fadeDirection = 1;
    uint8_t randomPosition = 0;
    const uint8_t fadeSteps = 50;
    uint8_t brightness = 255;

    void setBackgroundColor(uint32_t color);
    uint32_t applyGammaCorrection(uint32_t color);
    uint32_t applyBrightness(uint32_t color);
};