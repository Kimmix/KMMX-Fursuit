#include <sk.h>

#define SK_PIN 18
#define LED_COUNT 7

class SideLED {
   private:
    sk led;

   public:
    void init() {
        led.begin(SK_PIN, LED_COUNT);
        led.setbrightness(50);
    }

    void loop() {
        // set all LEDs to red
        // for (int i = 0; i < led.ledcount(); i++) {
        //     led.color(i, 255, 68, 108, 0);
        // }
        led.color(0, 255, 0, 0, 0);
        led.color(1, 0, 255, 0, 0);
        led.color(2, 0, 0, 255, 0);
        led.color(3, 255, 255, 0, 0);
        led.color(4, 0, 255, 255, 0);
        led.color(5, 255, 0, 255, 0);
        led.color(6, 255, 255, 0, 0);
        led.show();
    }
};
