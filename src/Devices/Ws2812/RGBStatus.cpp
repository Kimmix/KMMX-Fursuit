#include "RGBStatus.h"

RGBStatus::RGBStatus(uint16_t pin, uint8_t pixelCount)
    : pixel(pixelCount, pin, NEO_GRB + NEO_KHZ800), lastChangeTime(0), isOn(false) {}

void RGBStatus::init() {
    pixel.begin();
    pixel.show();  // Initialize all pixels to 'off'
}

void RGBStatus::setColor(uint8_t r, uint8_t g, uint8_t b) {
    setColorInternal(pixel.Color(r, g, b));
}

void RGBStatus::setColor(Color color) {
    switch (color) {
        case RED:
            setColor(255, 0, 0);
            break;
        case GREEN:
            setColor(0, 255, 0);
            break;
        case BLUE:
            setColor(0, 0, 255);
            break;
        case WHITE:
            setColor(255, 255, 255);
            break;
        case YELLOW:
            setColor(255, 255, 0);
            break;
        case CYAN:
            setColor(0, 255, 255);
            break;
        case MAGENTA:
            setColor(255, 0, 255);
            break;
    }
}

void RGBStatus::turnOff() {
    setColorInternal(pixel.Color(0, 0, 0));
    isOn = false;
}

void RGBStatus::update() {
    if (isOn && (millis() - lastChangeTime >= 2000)) {
        turnOff();
    }
}

void RGBStatus::setColorInternal(uint32_t color) {
    pixel.setPixelColor(0, color);
    pixel.show();
    lastChangeTime = millis();
    isOn = true;
}