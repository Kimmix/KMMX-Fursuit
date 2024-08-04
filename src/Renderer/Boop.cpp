#include "Boop.h"

float Boop::calculateBoopSpeed() {
    unsigned long elapsedTime = millis() - boopStartTime;
    float speed = map(elapsedTime, 100, BoopTimeMaxDuration, 0, 100);
    return speed / 100;
}

void Boop::getBoop(uint16_t& sensorValue, bool& isInRange, bool& isBoop, float& boopSpeed, bool& isContinuous, bool& isAngry) {
    isInRange = false;
    isBoop = false;
    isContinuous = false;
    isAngry = false;

    switch (currentBoopState) {
        case IDLE:
            if (sensorValue > IrMinThreshold && sensorValue < IrMaxThreshold) {
                currentBoopState = BOOP_IN_PROGRESS;
                boopStartTime = millis();
                isInRange = true;
            } else if (sensorValue >= 1023) {
                currentBoopState = ANGRY;
                isAngry = true;
            }
            break;

        case BOOP_IN_PROGRESS:
            isInRange = true;
            if (sensorValue >= IrMaxThreshold) {
                boopSpeed = calculateBoopSpeed();
                if (boopSpeed > 0.0) {
                    isBoop = true;
                    currentBoopState = BOOP_CONTINUOUS;
                    isContinuous = true;
                } else {
                    currentBoopState = IDLE;
                }
            } else if (sensorValue < IrMinThreshold) {
                currentBoopState = IDLE;
            }
            break;

        case BOOP_CONTINUOUS:
            isContinuous = true;
            if (sensorValue < IrMaxThreshold) {
                currentBoopState = IDLE;
            }
            break;

        case ANGRY:
            isAngry = true;
            if (sensorValue < IrMaxThreshold) {
                currentBoopState = IDLE;
            }
            break;
    }
}