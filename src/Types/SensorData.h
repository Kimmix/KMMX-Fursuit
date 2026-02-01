#pragma once

// Thread-safe sensor data structure for double-buffering
struct SensorData {
    float accelX = 0;
    float accelY = 0;
    float accelZ = 0;
    float accelMagnitude = 0;  // Cached magnitude for performance
    uint16_t proximity = 0;
};
