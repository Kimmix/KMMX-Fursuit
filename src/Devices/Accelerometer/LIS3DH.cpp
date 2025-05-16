#include "LIS3DH.h"

LIS3DH::LIS3DH() : lis(Adafruit_LIS3DH()) {}

void LIS3DH::setUp() {
    if (!lis.begin(0x18)) {
        Serial.println("Could not initialize LIS3DH");
    }
    lis.setDataRate(LIS3DH_DATARATE_100_HZ); // Higher data rate for lower latency
    lis.setRange(LIS3DH_RANGE_4_G);         // Wider range for better sensitivity
}

sensors_event_t* LIS3DH::getSensorEvent() {
    lis.getEvent(&event);
    updateMotionStatus(&event);
    return &event;
}

void LIS3DH::updateMotionStatus(sensors_event_t* event) {
    // Calculate magnitude of acceleration vector (squared for efficiency - avoid sqrt)
    float magnitudeSq =
        event->acceleration.x * event->acceleration.x +
        event->acceleration.y * event->acceleration.y +
        event->acceleration.z * event->acceleration.z;

    // Static gravity is approximately 9.8 m/s² squared = ~96.04
    // We're looking for deviation from this
    const float gravitySquared = 96.04f;  // 9.8^2
    float thresholdSquared = motionThreshold * motionThreshold;

    if (abs(magnitudeSq - gravitySquared) > thresholdSquared) {
        motionDetected = true;
        lastMotionTime = millis();
    } else if (millis() - lastMotionTime > 250) {
        motionDetected = false;
    }
}

bool LIS3DH::isMotionDetected() {
    return motionDetected;
}

void LIS3DH::setMotionThreshold(float threshold) {
    if (threshold > 0) {
        motionThreshold = threshold;
    }
}
