#include <FastLED.h>

#define BRIGHTNESS 255       // Set the initial brightness (0-255)
#define LED_TYPE WS2812      // WS2812 or WS2812B, depending on your LEDs
#define COLOR_ORDER GRB      // GRB or RGB, depending on your LEDs

class SideLED {
   public:
    SideLED() {
        FastLED.addLeds<LED_TYPE, ARGB, COLOR_ORDER>(leds, ARGB_NUM).setCorrection(TypicalLEDStrip);
        FastLED.setBrightness(BRIGHTNESS);
        for (short i = 0; i < ARGB_NUM; i++) {
            leds[i] = CHSV(150, 255, 200);
        }
    }

    void animate() {
        if (millis() >= prevTime) {
            prevTime = millis() + 500;
            updateLEDs();
            FastLED.show();
        }
    }

   private:
    CRGB leds[ARGB_NUM];
    const short steps = 24;
    short stepsCycle = 0, stepsCycleAlt = steps;
    unsigned long prevTime = 0;

    void updateLEDs() {
        // Define the new gradient colors in HSV
        CHSV color1HSV(239, 240, 135);  // Start color: #FF446C (Reddish Pink)
        CHSV color2HSV(22, 185, 125);   // End color: #F9826C (Coral)

        // Calculate the number of steps required for the hsv shift
        const int hueStep = ((color2HSV.h - color1HSV.h + 256) % 256) / steps;
        const int saturationStep = (color2HSV.s - color1HSV.s) / steps;
        const int valueStep = (color2HSV.v - color1HSV.v) / steps;

        stepsCycle = (stepsCycle + 1) % (steps / 2);
        int currentHue = (color1HSV.h + hueStep * stepsCycle + 256) % 256;  // Ensure the hue wraps around 256
        int currentSaturation = color1HSV.s + saturationStep * stepsCycle;
        int currentBrightness = color1HSV.v + valueStep * stepsCycle;
        leds[esp_random() % ARGB_NUM] = CHSV(currentHue, currentSaturation, currentBrightness);
        // Alternate
        stepsCycleAlt = ((stepsCycleAlt + 1) % (steps / 2)) + (steps / 2);
        int currentHueAlt = (color1HSV.h + hueStep * stepsCycleAlt + 256) % 256;  // Ensure the hue wraps around 256
        int currentSaturationAlt = color1HSV.s + saturationStep * stepsCycleAlt;
        int currentBrightnessAlt = color1HSV.v + valueStep * stepsCycleAlt;
        leds[esp_random() % ARGB_NUM] = CHSV(currentHueAlt, currentSaturationAlt, currentBrightnessAlt);
    }
};
