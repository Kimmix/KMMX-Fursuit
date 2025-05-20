#include "LIS3DH.h"

LIS3DH::LIS3DH() : lis(Adafruit_LIS3DH()) {}

void LIS3DH::setUp() {
    if (!lis.begin(0x18)) {
        Serial.println("Could not initialize LIS3DH");
    }
    lis.setDataRate(LIS3DH_DATARATE_25_HZ);
    lis.setRange(LIS3DH_RANGE_2_G);
}

sensors_event_t* LIS3DH::getSensorEvent() {
    lis.getEvent(&event);
    // if (DEBUG_LIS3DH) {
    //     Serial.print("Acceleration X: ");
    //     Serial.print(event.acceleration.x);
    //     Serial.println(" m/s^2");
    //     Serial.print("Acceleration Y: ");
    //     Serial.print(event.acceleration.y);
    //     Serial.println(" m/s^2");
    //     Serial.print("Acceleration Z: ");
    //     Serial.print(event.acceleration.z);
    //     Serial.println(" m/s^2");
    // }
    return &event;
}
