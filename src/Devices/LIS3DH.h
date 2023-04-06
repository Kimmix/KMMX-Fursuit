#include <Adafruit_LIS3DH.h>

// #define I2C_SDA GPIO_NUM_12
// #define I2C_SCL GPIO_NUM_13

class LIS3DH {
public:
    void init() {
        Serial.println("LIS3DH test!");
        // I2C.begin(I2C_SDA, I2C_SCL);
        if (!lis.begin(0x19)) {
            Serial.println("Couldnt LIS3DH");
            while (1) yield();
        }
        Serial.println("LIS3DH found!");
        // lis.setRange(LIS3DH_RANGE_4_G);
        // lis.setDataRate(LIS3DH_DATARATE_POWERDOWN);
    }

    void readAccelEvent(sensors_event_t* event) {
        lis.getEvent(event);
    }

    void readAccelG(float &x, float &y, float &z) {
        lis.read();
        x = lis.x_g;
        y = lis.y_g;
        z = lis.z_g;
    }
private:
    // TwoWire I2C = TwoWire(0);
    Adafruit_LIS3DH lis = Adafruit_LIS3DH();
};