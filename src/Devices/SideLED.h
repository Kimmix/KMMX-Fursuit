#include <Arduino.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_Sensor.h>
#include <FastLED.h>

#define LED_PIN GPIO_NUM_2  // Pin connected to the Data Input of WS2812
#define NUM_LEDS 7          // Number of WS2812 LEDs in your strip

#define BRIGHTNESS 40    // Set the initial brightness (0-255)
#define LED_TYPE WS2812  // WS2812 or WS2812B, depending on your LEDs
#define COLOR_ORDER GRB  // GRB or RGB, depending on your LEDs

CRGB leds[NUM_LEDS];

void bounceGradient() {
    static uint8_t startIndex = 0;  // The starting index of the gradient
    static bool forward = true;     // Flag to indicate the direction of the gradient

    // Define the new gradient colors in HSV
    CHSV color1HSV(0, 255, 255);   // Start color: #FF446C (Reddish Pink)
    CHSV color2HSV(20, 255, 255);  // End color: #F9826C (Coral)

    // Calculate the step for each color component
    int hueStep = (color2HSV.h - color1HSV.h) / NUM_LEDS;
    int saturationStep = (color2HSV.s - color1HSV.s) / NUM_LEDS;
    int valueStep = (color2HSV.v - color1HSV.v) / NUM_LEDS;

    // Fill the LED strip with the gradient
    for (int i = 0; i < NUM_LEDS; i++) {
        int index = (startIndex + i) % NUM_LEDS;  // Calculate the current index for the gradient
        CHSV colorHSV(color1HSV.h + hueStep * i, color1HSV.s + saturationStep * i, color1HSV.v + valueStep * i);
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
        if (startIndex <= 0) {
            forward = true;  // Change direction when reaching the beginning
            startIndex = 0;
        }
    }
}

void setup() {
    Serial.begin(115200);
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
    FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
    bounceGradient();
    FastLED.show();
    delay(100);
}
