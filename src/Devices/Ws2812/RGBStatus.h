#pragma once

// Default to enabled if not set by platformio.ini
#ifndef ENABLE_RGB_STATUS_LED
#define ENABLE_RGB_STATUS_LED 1
#endif

#if ENABLE_RGB_STATUS_LED
#include <Adafruit_NeoPixel.h>
#include "GammaTable.h"
#endif

enum Color { RED,
             PINK,
             LIGHT_PINK,
             BLUE,
             WHITE,
             YELLOW,
             CYAN,
             MAGENTA };

class RGBStatus {
   public:
    RGBStatus(int pin, int pixelCount = 1);
    void init();
    void setColor(int r, int g, int b);
    void setColor(Color color);
    void turnOff();
    void update();

   private:
#if ENABLE_RGB_STATUS_LED
    Adafruit_NeoPixel pixel;
    unsigned long lastChangeTime;
    bool isOn;

    void setColorInternal(uint32_t color);
#else
    // No-op implementation when disabled (zero storage overhead)
    int dummyPin;
#endif
};
