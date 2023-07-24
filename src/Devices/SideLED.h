#include <Adafruit_BusIO_Register.h>
#include <Adafruit_Sensor.h>
#include <FastLED.h>

#define LED_PIN GPIO_NUM_18  // Pin connected to the Data Input of WS2812
#define NUM_LEDS 7          // Number of WS2812 LEDs in your strip
#define BRIGHTNESS 10        // Set the initial brightness (0-255)
#define LED_TYPE WS2812     // WS2812 or WS2812B, depending on your LEDs
#define COLOR_ORDER GRB     // GRB or RGB, depending on your LEDs

class SideLED {
   public:
    SideLED() : startIndex(0), forward(true) {
        FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
        FastLED.setBrightness(BRIGHTNESS);
    }

    void animate() {
        // Define the new gradient colors in HSV
        CHSV color1HSV(239, 240, 135);  // Start color: #FF446C (Reddish Pink)
        CHSV color2HSV(22, 185, 125);   // End color: #F9826C (Coral)

        // Calculate the number of steps required for the hue shift
        int hueDistance = (color2HSV.h - color1HSV.h + 256) % 256;
        int hueStep = hueDistance / NUM_LEDS;

        // Calculate the step for each color component
        int saturationStep = (color2HSV.s - color1HSV.s) / NUM_LEDS;
        int valueStep = (color2HSV.v - color1HSV.v) / NUM_LEDS;

        // Calculate the time-based animation speed
        unsigned long currentTime = millis();
        if (currentTime - prevTime >= 250) {
            prevTime = currentTime;
            // Fill the LED strip with the gradient
            for (int i = 0; i < NUM_LEDS; i++) {
                int index = (startIndex + i) % NUM_LEDS;                   // Calculate the current index for the gradient
                int currentHue = (color1HSV.h + hueStep * i + 256) % 256;  // Ensure the hue wraps around 256
                CHSV colorHSV(currentHue, color1HSV.s + saturationStep * i, color1HSV.v + valueStep * i);
                CRGB colorRGB;
                hsv2rgb_rainbow(colorHSV, colorRGB);
                leds[index] = colorRGB;
            }

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
        FastLED.show();
    }

   private:
    CRGB leds[NUM_LEDS];
    uint8_t startIndex;
    bool forward;
    unsigned long prevTime = 0;
};