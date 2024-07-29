#include "Boop.h"

float Boop::calculateBoopSpeed() {
    unsigned long elapsedTime = millis() - boopStartTime;
    float speed = map(elapsedTime, 100, BoopTimeMaxDuration, 0, 100);
    return speed / 100;
}

void Boop::getBoop(uint16_t& sensorValue, bool& isInRange, bool& isBoop, float& boopSpeed, bool& isContinuous, bool& isAngry) {
    switch (currentBoopState) {
        case IDLE:
            isInRange = false;
            isBoop = false;
            isContinuous = false;
            isAngry = false;
            if (sensorValue > IrMinThreshold && sensorValue < IrMaxThreshold) {
                currentBoopState = BOOP_IN_PROGRESS;
                boopStartTime = millis();
            } else if (sensorValue >= 1023) {
                currentBoopState = ANGRY;
            }
            break;
        case BOOP_IN_PROGRESS:
            isInRange = true;
            isBoop = false;
            isContinuous = false;
            isAngry = false;
            if (sensorValue >= IrMaxThreshold) {
                boopSpeed = calculateBoopSpeed();
                if (boopSpeed > 0.0) {
                    isBoop = true;
                    if (sensorValue >= IrMaxThreshold) {
                        currentBoopState = BOOP_CONTINUOUS;
                        break;
                    }
                }
                currentBoopState = IDLE;
            } else if (sensorValue < IrMinThreshold) {
                currentBoopState = IDLE;
            }
            break;
        case BOOP_CONTINUOUS:
            isInRange = false;
            isBoop = false;
            isContinuous = true;
            isAngry = false;
            if (sensorValue < IrMaxThreshold) {
                currentBoopState = IDLE;
            }
            break;
        case ANGRY:
            isInRange = false;
            isBoop = false;
            isContinuous = false;
            isAngry = true;
            if (sensorValue < IrMaxThreshold) {
                currentBoopState = IDLE;
            }
            break;
    }
}
