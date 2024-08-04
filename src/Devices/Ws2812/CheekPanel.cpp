#include "CheekPanel.h"

CheekPanel::CheekPanel(uint16_t numLeds, uint8_t pin)
    : strip(numLeds, pin, NEO_GRB + NEO_KHZ800) {}

void CheekPanel::configure(uint32_t backgroundColor, uint32_t fadeColor, uint16_t fadeInterval, uint16_t positionChangeDelay) {
    this->backgroundColor = backgroundColor;
    this->fadeColor = fadeColor;
    this->fadeInterval = fadeInterval;
    this->positionChangeDelay = positionChangeDelay;
}

void CheekPanel::init() {
    strip.begin();
    strip.show();  // Initialize all pixels to 'off'
    setBackgroundColor(applyGammaCorrection(backgroundColor));
    previousMillis = millis();
    positionChangeMillis = millis();  // Initialize the position change timer
}

void CheekPanel::update() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= fadeInterval) {
        previousMillis = currentMillis;
        fadeStep += fadeDirection;

        if (fadeStep >= fadeSteps) {
            fadeDirection = -1;
        } else if (fadeStep <= 0) {
            fadeDirection = 1;

            // Check if the delay before picking a new position has elapsed
            if (currentMillis - positionChangeMillis >= positionChangeDelay) {
                randomPosition = random(strip.numPixels());
                positionChangeMillis = currentMillis;  // Reset the position change timer
            }
        }

        float factor = (float)fadeStep / fadeSteps;
        uint32_t fadeColorCorrected = strip.Color(
            (uint8_t)(((fadeColor >> 16) & 0xFF) * factor),
            (uint8_t)(((fadeColor >> 8) & 0xFF) * factor),
            (uint8_t)((fadeColor & 0xFF) * factor));

        // Set the background color for all pixels
        setBackgroundColor(applyGammaCorrection(backgroundColor));

        // Set the random position and surrounding pixels' colors with gamma correction and wrap around
        for (int i = -2; i <= 2; i++) {
            int pos = (randomPosition + i + strip.numPixels()) % strip.numPixels();
            strip.setPixelColor(pos, applyGammaCorrection(fadeColorCorrected));
        }

        strip.show();
    }
}

void CheekPanel::setBackgroundColor(uint32_t color) {
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, color);
    }
}

uint32_t CheekPanel::applyGammaCorrection(uint32_t color) {
    uint8_t r = pgm_read_byte(&gammaTable[(color >> 16) & 0xFF]);
    uint8_t g = pgm_read_byte(&gammaTable[(color >> 8) & 0xFF]);
    uint8_t b = pgm_read_byte(&gammaTable[color & 0xFF]);
    return strip.Color(r, g, b);
}
