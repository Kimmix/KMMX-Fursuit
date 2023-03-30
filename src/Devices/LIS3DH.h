#include <Adafruit_LIS3DH.h>

#define I2C_SDA GPIO_NUM_12
#define I2C_SCL GPIO_NUM_13

class LIS3DH {
public:
    void init() {
        Serial.println("LIS3DH test!");
        delay(1000);
        I2C.begin(I2C_SDA, I2C_SCL);
        delay(1000);
        if (!lis.begin(0x19)) {   // change this to 0x19 for alternative i2c address
            Serial.println("Couldnt LIS3DH");
            while (1) yield();
        }
        Serial.println("LIS3DH found!");
        Serial.print("Range = "); Serial.print(2 << lis.getRange());
        Serial.println("G");

        Serial.print("Data rate set to: ");
        switch (lis.getDataRate()) {
        case LIS3DH_DATARATE_1_HZ: Serial.println("1 Hz"); break;
        case LIS3DH_DATARATE_10_HZ: Serial.println("10 Hz"); break;
        case LIS3DH_DATARATE_25_HZ: Serial.println("25 Hz"); break;
        case LIS3DH_DATARATE_50_HZ: Serial.println("50 Hz"); break;
        case LIS3DH_DATARATE_100_HZ: Serial.println("100 Hz"); break;
        case LIS3DH_DATARATE_200_HZ: Serial.println("200 Hz"); break;
        case LIS3DH_DATARATE_400_HZ: Serial.println("400 Hz"); break;
        case LIS3DH_DATARATE_POWERDOWN: Serial.println("Powered Down"); break;
        case LIS3DH_DATARATE_LOWPOWER_5KHZ: Serial.println("5 Khz Low Power"); break;
        case LIS3DH_DATARATE_LOWPOWER_1K6HZ: Serial.println("16 Khz Low Power"); break;
        }
    }

    sensors_event_t readAccel() {
        sensors_event_t event;
        lis.getEvent(&event);
        return event;
    }
private:
    TwoWire I2C = TwoWire(0);
    Adafruit_LIS3DH lis = Adafruit_LIS3DH(&I2C);
};