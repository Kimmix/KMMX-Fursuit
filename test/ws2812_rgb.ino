#include <Adafruit_NeoPixel.h>

// Configuration for a single RGB NeoPixel
#define SINGLE_RGB_PIN 45
#define SINGLE_RGB_COUNT 1
Adafruit_NeoPixel singleRgbPixel(SINGLE_RGB_COUNT, SINGLE_RGB_PIN, NEO_GRB + NEO_KHZ800);

// Configuration for ARGB Strip
#define ARGB_PIN 14
#define ARGB_COUNT 24  // Change this to the number of ARGB LEDs you have
Adafruit_NeoPixel argbStrip(ARGB_COUNT, ARGB_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    // Initialize the single RGB NeoPixel
    singleRgbPixel.begin();
    singleRgbPixel.show();  // Initialize to 'off'

    // Initialize the ARGB Strip
    argbStrip.begin();
    argbStrip.show();  // Initialize to 'off'
}

void loop() {
    // Control the single RGB NeoPixel
    singleRgbPixel.setPixelColor(0, singleRgbPixel.Color(255, 0, 0));  // Red color
    singleRgbPixel.show();
    delay(1000);

    singleRgbPixel.setPixelColor(0, singleRgbPixel.Color(0, 255, 0));  // Green color
    singleRgbPixel.show();
    delay(1000);

    singleRgbPixel.setPixelColor(0, singleRgbPixel.Color(0, 0, 255));  // Blue color
    singleRgbPixel.show();
    delay(1000);

    // Control the ARGB Strip
    colorWipe(argbStrip, 255, 0, 0, 50);  // Red color wipe
    colorWipe(argbStrip, 0, 255, 0, 50);  // Green color wipe
    colorWipe(argbStrip, 0, 0, 255, 50);  // Blue color wipe
}

// Function to perform a color wipe on the ARGB strip
void colorWipe(Adafruit_NeoPixel& strip, uint8_t red, uint8_t green, uint8_t blue, int wait) {
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(red, green, blue));
        strip.show();
        delay(wait);
    }
}
