#pragma once

#include <Adafruit_NeoPixel.h>
#include "GammaTable.h"

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
    Adafruit_NeoPixel pixel;
    unsigned long lastChangeTime;
    bool isOn;

    void setColorInternal(uint32_t color);
};
