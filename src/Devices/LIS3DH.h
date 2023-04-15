#include <Adafruit_LIS3DH.h>

// #define I2C_SDA GPIO_NUM_12
// #define I2C_SCL GPIO_NUM_13

// Adjust this number for the sensitivity of the 'click' force
// this strongly depend on the range! for 16G, try 5-10
// for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
#define CLICKTHRESHHOLD 120

class LIS3DH {
   public:
    void init() {
        // I2C.begin(I2C_SDA, I2C_SCL);
        if (!lis.begin(0x19)) {
            Serial.println("Couldnt LIS3DH");
            while (1) yield();
        }
        Serial.println("LIS3DH found!");
        // lis.setRange(LIS3DH_RANGE_4_G);
        // lis.setDataRate(LIS3DH_DATARATE_POWERDOWN);

        // 0 = turn off click detection & interrupt
        // 1 = single click only interrupt output
        // 2 = double click only interrupt output, detect single click
        // Adjust threshhold, higher numbers are less sensitive
        lis.setClick(2, CLICKTHRESHHOLD);
    }

    void readAccelEvent(sensors_event_t *event) {
        lis.getEvent(event);
    }

    void readAccelG(float &x, float &y, float &z) {
        lis.read();
        x = lis.x_g;
        y = lis.y_g;
        z = lis.z_g;
    }

    bool isTap() {
        if (lis.getClick() & 0x20) return true;
        return false;
    }

    //? Debug funtion
    void getTap() {
        uint8_t click = lis.getClick();
        if (click == 0) return;
        if (!(click & 0x30)) return;
        Serial.print("Click detected (0x");
        Serial.print(click, HEX);
        Serial.print("): ");
        if (click & 0x10) Serial.print(" single click");
        if (click & 0x20) Serial.print(" double click");
        Serial.println();
    }

   private:
    // TwoWire I2C = TwoWire(0);
    Adafruit_LIS3DH lis = Adafruit_LIS3DH();
};