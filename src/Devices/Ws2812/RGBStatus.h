#pragma once

#include <Adafruit_NeoPixel.h>

enum Color { RED, GREEN, BLUE, WHITE, YELLOW, CYAN, MAGENTA };

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

    void setColorInternal(int color);
};
