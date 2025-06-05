#include "LIS3DH.h"

LIS3DH::LIS3DH() : lis(Adafruit_LIS3DH()) {}

void LIS3DH::setUp() {
    if (!lis.begin(0x18)) {
        Serial.println("Could not initialize LIS3DH");
    }
    lis.setDataRate(LIS3DH_DATARATE_25_HZ);
    lis.setRange(LIS3DH_RANGE_2_G);
}

void LIS3DH::setDataRate(uint8_t rate) {
    lis.setDataRate((lis3dh_dataRate_t)rate);
}

sensors_event_t* LIS3DH::getSensorEvent() {
    lis.getEvent(&event);
    return &event;
}
