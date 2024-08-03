#include "CheekPanel.h"

CheekPanel::CheekPanel(uint16_t pin, uint16_t numLeds)
    : strip(numLeds, pin, NEO_GRB + NEO_KHZ800), numLeds(numLeds), hue1(15000), hue2(42000), offset(0), lastUpdateTime(0) {}

void CheekPanel::init() {
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
}

void CheekPanel::update() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdateTime >= 50) { // Update rate in milliseconds
        for (uint16_t i = 0; i < numLeds; i++) {
            float ratio = (sin((i + offset) * 2 * PI / numLeds) + 1) / 2;
            uint16_t hue = hue1 * (1 - ratio) + hue2 * ratio;
            strip.setPixelColor(i, colorFromHue(hue));
        }
        strip.show();
        offset += 0.1; // Adjust speed of color transition
        if (offset >= numLeds) offset = 0;
        lastUpdateTime = currentMillis;
    }
}

uint32_t CheekPanel::colorFromHue(uint16_t hue) {
    float r, g, b;
    float s = 1.0;
    float v = 1.0;

    int i = hue / 65536;
    float f = hue / 65536.0 - i;
    float p = v * (1 - s);
    float q = v * (1 - s * f);
    float t = v * (1 - s * (1 - f));

    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    return strip.Color(r * 255, g * 255, b * 255);
}
