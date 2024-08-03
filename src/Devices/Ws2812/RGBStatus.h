#pragma once

#include <Adafruit_NeoPixel.h>

enum Color { RED, GREEN, BLUE, WHITE, YELLOW, CYAN, MAGENTA };

class RGBStatus {
public:
    RGBStatus(uint16_t pin, uint8_t pixelCount = 1);
    void init();
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void setColor(Color color);
    void turnOff();
    void update();

private:
    Adafruit_NeoPixel pixel;
    unsigned long lastChangeTime;
    bool isOn;

    void setColorInternal(uint32_t color);
};
