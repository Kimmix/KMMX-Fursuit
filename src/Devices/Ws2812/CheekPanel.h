#pragma once

#include <Adafruit_NeoPixel.h>
#include "GammaTable.h"

// Optimized circular edge LED strip - Color Wave effect with easing
// Color expands smoothly from random position, respects L/R split
class CheekPanel {
   public:
    CheekPanel(uint16_t numLeds, uint8_t pin);
    void configure(uint32_t color1, uint32_t color2, uint16_t updateInterval);
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
    uint32_t color1;
    uint32_t color2;
    uint16_t updateInterval;
    unsigned long previousMillis;
    uint8_t animStep;          // Animation step (0-39 for ~8s cycle)
    int8_t animDirection;      // 1 = expanding, -1 = contracting
    uint8_t centerPos;         // Center position for expansion
    uint8_t brightness;

    uint32_t applyGamma(uint32_t color);
    uint8_t easeInOutQuad(uint8_t t);  // Smooth easing function
};