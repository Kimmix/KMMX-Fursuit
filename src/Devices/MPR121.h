#include "Adafruit_MPR121.h"

class MPR121 {
   private:
    Adafruit_MPR121 cap = Adafruit_MPR121();
    // Keeps track of the last pins touched
    // so we know when buttons are 'released'
    uint16_t lasttouched = 0;
    uint16_t currtouched = 0;

   public:
    void init() {
        if (!cap.begin(0x5A)) {
            Serial.println("Couldnt LIS3DH");
            while (1)
                ;
        }
        Serial.println("MPR121 found!");
    }

    void loop() {
        // Get the currently touched pads
        currtouched = cap.touched();

        for (uint8_t i = 0; i < 12; i++) {
            // it if *is* touched and *wasnt* touched before, alert!
            if ((currtouched & _BV(i)) && !(lasttouched & _BV(i))) {
                Serial.print(i);
                Serial.println(" touched");
            }
            // if it *was* touched and now *isnt*, alert!
            if (!(currtouched & _BV(i)) && (lasttouched & _BV(i))) {
                Serial.print(i);
                Serial.println(" released");
            }
        }

        // reset our state
        lasttouched = currtouched;

        // comment out this line for detailed data from the sensor!
        return;

        // debugging info, what
        Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x");
        Serial.println(cap.touched(), HEX);
        Serial.print("Filt: ");
        for (uint8_t i = 0; i < 12; i++) {
            Serial.print(cap.filteredData(i));
            Serial.print("\t");
        }
        Serial.println();
        Serial.print("Base: ");
        for (uint8_t i = 0; i < 12; i++) {
            Serial.print(cap.baselineData(i));
            Serial.print("\t");
        }
        Serial.println();

        // put a delay so it isn't overwhelming
        delay(100);
    }
};
