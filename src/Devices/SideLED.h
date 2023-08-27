#include <FastLED.h>

#define LED_PIN GPIO_NUM_18  // Pin connected to the Data Input of WS2812
#define NUM_LEDS 7           // Number of WS2812 LEDs in your strip
#define BRIGHTNESS 255       // Set the initial brightness (0-255)
#define LED_TYPE WS2812      // WS2812 or WS2812B, depending on your LEDs
#define COLOR_ORDER GRB      // GRB or RGB, depending on your LEDs
// https://gist.github.com/atuline/030e9cc163c5067ae04cf63a604c4eb7
const uint8_t PROGMEM gamma8[] = {
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

class SideLED {
   public:
    SideLED() {
        FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
        // FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(0xFFB0F0);
        FastLED.setBrightness(BRIGHTNESS);
    }

    void animate() {
        unsigned long currentTime = millis();
        if (currentTime - prevTime >= 250) {
            prevTime = currentTime;
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

        // Calculate the number of steps required for the hue shift
        int hueDistance = (color2HSV.h - color1HSV.h + 256) % 256;
        int hueStep = hueDistance / NUM_LEDS;

        // Calculate the step for each color component
        int saturationStep = (color2HSV.s - color1HSV.s) / NUM_LEDS;
        int valueStep = (color2HSV.v - color1HSV.v) / NUM_LEDS;

        // Fill the LED strip with the gradient
        for (int i = 0; i < NUM_LEDS; i++) {
            int index = (startIndex + i) % NUM_LEDS;                   // Calculate the current index for the gradient
            int currentHue = (color1HSV.h + hueStep * i + 256) % 256;  // Ensure the hue wraps around 256
            // CHSV colorHSV(currentHue, color1HSV.s + saturationStep * i, color1HSV.v + valueStep * i);
            // CRGB colorRGB;
            // hsv2rgb_rainbow(colorHSV, colorRGB);
            // leds[index] = colorRGB;
            leds[index] = CHSV(currentHue, color1HSV.s + (saturationStep * i), color1HSV.v + (valueStep * i));
            // uint8_t fader = sin8(millis() / random8(10, 20));
            // fader = pgm_read_byte(&gamma8[fader]);
            // leds[i] = CHSV(i * 20, 255, fader);
        }
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
