#include <FastLED.h>

#define LED_PIN GPIO_NUM_18  // Pin connected to the Data Input of WS2812
#define NUM_LEDS 7           // Number of WS2812 LEDs in your strip
#define BRIGHTNESS 255       // Set the initial brightness (0-255)
#define LED_TYPE WS2812      // WS2812 or WS2812B, depending on your LEDs
#define COLOR_ORDER GRB      // GRB or RGB, depending on your LEDs

class SideLED {
   public:
    SideLED() {
        FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
        FastLED.setBrightness(BRIGHTNESS);
    }

    void animate() {
        if (millis() >= prevTime) {
            prevTime = millis() + 200;
            updateLEDs();
            moveIndex();
            FastLED.show();
        }
    }

   private:
    CRGB leds[NUM_LEDS];
    uint8_t startIndex = 0;
    bool forward = true;
    unsigned long prevTime = 0;

    void updateLEDs() {
        // Define the new gradient colors in HSV
        CHSV color1HSV(239, 240, 135);  // Start color: #FF446C (Reddish Pink)
        CHSV color2HSV(22, 185, 125);   // End color: #F9826C (Coral)

        // Calculate the number of steps required for the hsv shift
        int hueStep = ((color2HSV.h - color1HSV.h + 256) % 256) / NUM_LEDS;
        int saturationStep = (color2HSV.s - color1HSV.s) / NUM_LEDS;
        int valueStep = (color2HSV.v - color1HSV.v) / NUM_LEDS;

        // Fill the LED strip with the gradient
        for (int i = 0; i < NUM_LEDS; i++) {
            int index = (startIndex + i) % NUM_LEDS;                   // Calculate the current index for the gradient
            int currentHue = (color1HSV.h + hueStep * i + 256) % 256;  // Ensure the hue wraps around 256
            int currentSaturation = color1HSV.s + saturationStep * i;
            int currentBrightness = color1HSV.v + valueStep * i;
            leds[index] = CHSV(currentHue, currentSaturation, currentBrightness);
        }
        // Fill one esp_random()
    }

    void moveIndex() {
        // Move the starting index based on the direction
        if (forward) {
            startIndex++;
            if (startIndex >= NUM_LEDS) {
                forward = false;  // Change direction when reaching the end
                startIndex = NUM_LEDS - 1;
            }
        } else {
            startIndex--;
            if (startIndex < 0) {
                forward = true;  // Change direction when reaching the beginning
                startIndex = 0;
            }
        }
    }
};
