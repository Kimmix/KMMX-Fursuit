#include <FastLED.h>

#define LED_TYPE WS2812  // WS2812 or WS2812B, depending on your LEDs
#define COLOR_ORDER GRB  // GRB or RGB, depending on your LEDs
#define ARGB_NUM 24
class SideLED {
   public:
    SideLED() {
        FastLED.addLeds<LED_TYPE, ARGB, COLOR_ORDER>(leds, ARGB_NUM).setCorrection(TypicalLEDStrip);
        FastLED.setBrightness(brightness);
    }

    void animate() {
        static uint8_t startIndex = 0;
        startIndex = startIndex + 1; /* motion speed */
        if (millis() >= prevTime) {
            prevTime = millis() + 400;
            FillLEDsFromPaletteColors(startIndex);
            FastLED.show();
        }
    }

   private:
    CRGB leds[ARGB_NUM];
    const short brightness = 255;
    unsigned long prevTime = 0;

    void FillLEDsFromPaletteColors(uint8_t colorIndex) {
        for (int i = 0; i < ARGB_NUM; i++) {
            // Get the target color from the palette
            CRGB targetColor = ColorFromPalette(lavaLampPalette(), colorIndex, brightness, LINEARBLEND);
            // Interpolate between the current color and the target color
            leds[i] = blend(leds[i], targetColor, 128);
            colorIndex += 3;  // Increment the index by the calculated delta
        }
    }

    CRGBPalette16 lavaLampPalette() {
        CHSV color1HSV(239, 240, 135);  // Start color: #FF446C (Reddish Pink)
        CHSV color2HSV(22, 185, 125);   // End color: #F9826C (Coral)
        return CRGBPalette16(
            color1HSV, color2HSV, color1HSV, color2HSV,
            color1HSV, color2HSV, color1HSV, color2HSV,
            color1HSV, color2HSV, color1HSV, color2HSV,
            color1HSV, color2HSV, color1HSV, color2HSV);
    }
};
