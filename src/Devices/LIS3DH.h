#include <Adafruit_LIS3DH.h>

Adafruit_LIS3DH lis = Adafruit_LIS3DH();
class LIS3DH {
   private:
    sensors_event_t event;

   public:
    void setUp() {
        if (!lis.begin(0x18)) {
            Serial.println("Could not initialize LIS3DH");
        }
        lis.setDataRate(LIS3DH_DATARATE_25_HZ);
        lis.setRange(LIS3DH_RANGE_2_G);
    }

    sensors_event_t* getSensorEvent() {
        lis.getEvent(&event);
        return &event;
    }
};
