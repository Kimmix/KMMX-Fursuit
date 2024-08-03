#include "CheekPanel.h"

// Gamma correction lookup table (precomputed for gamma ~2.2)
const int CheekPanel::gammaTable[256] PROGMEM = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
    2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5,
    5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
    10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
    17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
    25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
    37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
    51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
    69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
    90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
    115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
    144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
    177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
    215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255};

CheekPanel::CheekPanel(int pin, int numLeds)
    : pin(pin), numLeds(numLeds), strip(numLeds, pin, NEO_GRB + NEO_KHZ800), previousMillis(0), interval(1000), shimmerIncrement(5) {}

void CheekPanel::init() {
    strip.begin();
    strip.show();  // Initialize all pixels to 'off'
}

void CheekPanel::update() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        static uint32_t startColor = strip.Color(255, 68, 108);  // Start color (CSS rgba(255,68,108,1))
        static uint32_t endColor = strip.Color(249, 130, 108);   // End color (CSS rgba(249,130,108,1))

        // Shift colors by one position (like a lava lamp)
        uint32_t tempColor = startColor;
        startColor = endColor;
        endColor = tempColor;

        // Add a subtle shimmer (randomly adjust brightness)
        for (int i = 0; i < numLeds; i++) {
            float t = float(i) / float(numLeds - 1);  // Interpolation factor (0.0 to 1.0)
            int r = lerpColorComponent((startColor >> 16) & 0xFF, (endColor >> 16) & 0xFF, t);
            int g = lerpColorComponent((startColor >> 8) & 0xFF, (endColor >> 8) & 0xFF, t);
            int b = lerpColorComponent(startColor & 0xFF, endColor & 0xFF, t);

            // Apply gamma correction
            r = gammaCorrection(r);
            g = gammaCorrection(g);
            b = gammaCorrection(b);

            // Add random shimmer (adjust brightness up or down)
            // Randomly choose whether to increase or decrease brightness
            int shimmerDirection = esp_random() % 2 == 0 ? -1 : 1;
            shimmerIncrement += shimmerDirection;
            shimmerIncrement = max(-10, min(10, shimmerIncrement));

            // Update color values
            r = min(255, r + shimmerIncrement);
            g = min(255, g + shimmerIncrement);
            b = min(255, b + shimmerIncrement);

            uint32_t color = strip.Color(r, g, b);
            strip.setPixelColor(i, color);
        }

        strip.show();
    }
}

int CheekPanel::lerpColorComponent(int startValue, int endValue, float t) {
    return int(startValue + t * (endValue - startValue));
}

int CheekPanel::gammaCorrection(int value) {
    return pgm_read_byte(&gammaTable[value]);
}
