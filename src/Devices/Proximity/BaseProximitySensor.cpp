#include "BaseProximitySensor.h"
#include "Utils/Utils.h"

/**
 * Add a proximity reading to the circular buffer.
 */
void BaseProximitySensor::addProximityToBuffer(uint16_t value) {
    proximityBuffer[bufferIndex] = value;
    bufferIndex = (bufferIndex + 1) % AVERAGING_SAMPLES;

    if (bufferIndex == 0) {
        bufferFilled = true;
    }
}

/**
 * Apply median filter to proximity readings for noise rejection.
 * Uses the optimized generic medianFilter from Utils.
 */
uint16_t BaseProximitySensor::medianFilter() {
    if (!bufferFilled && bufferIndex == 0) {
        return 0; // No data yet
    }

    int count = bufferFilled ? AVERAGING_SAMPLES : bufferIndex;

    // Use optimized generic median filter from Utils
    return ::medianFilter<uint16_t, AVERAGING_SAMPLES>(proximityBuffer, count);
}

/**
 * Initialize the proximity buffer with zeros.
 */
void BaseProximitySensor::initializeBuffer() {
    for (int i = 0; i < AVERAGING_SAMPLES; i++) {
        proximityBuffer[i] = 0;
    }
    bufferIndex = 0;
    bufferFilled = false;
    readSkipCounter = 0;
    cachedProximity = 0;
    lastDebugTime = 0;
}

/**
 * Check if enough time has passed for debug output.
 */
bool BaseProximitySensor::shouldPrintDebug() {
    unsigned long currentTime = millis();
    if (currentTime - lastDebugTime >= DEBUG_INTERVAL) {
        lastDebugTime = currentTime;
        return true;
    }
    return false;
}
