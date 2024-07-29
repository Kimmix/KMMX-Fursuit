#include "ARGBStrip.h"

ARGBStrip::ARGBStrip() {
    FastLED.addLeds<LED_TYPE, ARGB_PIN, COLOR_ORDER>(leds, argbNum)
        .setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(sideLEDBrightness);
}

void ARGBStrip::animate() {
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    if (millis() >= prevTime) {
        prevTime = millis() + sideLEDAnimateInterval;
        FillLEDsFromPaletteColors(startIndex);
        FastLED.show();
    }
}

void ARGBStrip::FillLEDsFromPaletteColors(uint8_t colorIndex) {
    for (int i = 0; i < argbNum; i++) {
        // Get the target color from the palette
        CRGB targetColor = ColorFromPalette(lavaLampPalette(), colorIndex, sideLEDBrightness, LINEARBLEND);
        // Interpolate between the current color and the target color
        leds[i] = blend(leds[i], targetColor, 128);
        colorIndex += 3;  // Increment the index by the calculated delta
    }
}

CRGBPalette16 ARGBStrip::lavaLampPalette() {
    return CRGBPalette16(
        sideColor1HSV, sideColor2HSV, sideColor1HSV, sideColor2HSV,
        sideColor1HSV, sideColor2HSV, sideColor1HSV, sideColor2HSV,
        sideColor1HSV, sideColor2HSV, sideColor1HSV, sideColor2HSV,
        sideColor1HSV, sideColor2HSV, sideColor1HSV, sideColor2HSV);
}
