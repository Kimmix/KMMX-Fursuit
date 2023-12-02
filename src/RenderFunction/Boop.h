enum BoopState {
    IDLE,
    BOOP_IN_PROGRESS,
    BOOP_CONTINUOUS,
    ANGRY
};

class Boop {
   private:
    BoopState currentBoopState = IDLE;
    const int IR_IN_RANGE_THRESHOLD = 100, IR_OUT_RANGE_THRESHOLD = 900;
    const unsigned long boopDuration = 1000;
    unsigned long boopStartTime = 0;
    bool isSensorStarted = false;

    float calculateBoopSpeed() {
        unsigned long elapsedTime = millis() - boopStartTime;
        float speed = map(elapsedTime, 100, boopDuration, 0, 100);
        return speed / 100;
    }

   public:
    void getBoop(uint16_t& sensorValue, bool& isInRange, bool& isBoop, float& boopSpeed, bool& isContinuous, bool& isAngry) {
        isInRange = false;
        // Serial.println(sensorValue);
        switch (currentBoopState) {
            case IDLE:
                if (sensorValue > IR_IN_RANGE_THRESHOLD && sensorValue < IR_OUT_RANGE_THRESHOLD) {
                    boopStartTime = millis();
                    currentBoopState = BOOP_IN_PROGRESS;
                } else if (sensorValue >= 1023) {
                    currentBoopState = ANGRY;
                }
                isBoop = false;
                isContinuous = false;
                isAngry = false;
                break;
            case BOOP_IN_PROGRESS:
                isInRange = true;
                if (sensorValue >= IR_OUT_RANGE_THRESHOLD) {
                    boopSpeed = calculateBoopSpeed();
                    if (boopSpeed > 0.0) {
                        isBoop = true;
                        if (sensorValue >= IR_OUT_RANGE_THRESHOLD) {
                            currentBoopState = BOOP_CONTINUOUS;
                            break;
                        }
                    }
                    currentBoopState = IDLE;
                } else if (sensorValue < IR_IN_RANGE_THRESHOLD) {
                    currentBoopState = IDLE;
                }
                break;
            case BOOP_CONTINUOUS:
                isBoop = false;
                isContinuous = true;
                isAngry = false;
                if (sensorValue < IR_IN_RANGE_THRESHOLD) {
                    currentBoopState = IDLE;
                }
                break;
            case ANGRY:
                isBoop = false;
                isContinuous = false;
                isAngry = true;
                if (sensorValue < IR_IN_RANGE_THRESHOLD) {
                    currentBoopState = IDLE;
                }
                break;
        }
    }
};
