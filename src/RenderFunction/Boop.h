#include "Devices/APDS9930Sensor.h"

#define IR_PIN GPIO_NUM_35
enum BoopState {
    IDLE,
    BOOP_IN_PROGRESS,
};

class Boop {
   private:
    APDS9930Sensor prox;
    BoopState currentBoopState = IDLE;
    const int IR_IN_RANGE_THRESHOLD = 1000, IR_OUT_RANGE_THRESHOLD = 3500;
    const unsigned long boopDuration = 1000;
    unsigned long boopStartTime = 0;
    bool isSensorStarted = false;

    float calculateBoopSpeed() {
        unsigned long elapsedTime = millis() - boopStartTime;
        float speed = map(elapsedTime, 100, boopDuration, 0, 100);
        return speed / 100;
    }

   public:
    void getBoop(bool& isInRange, bool& isBoop, float& boopSpeed) {
        // if (!isSensorStarted) {
        //     isSensorStarted = true;
        //     prox.setup();
        // }
        int sensorValue = analogRead(IR_PIN);
        // int sensorValue = prox.read();
        isInRange = false;
        switch (currentBoopState) {
            case IDLE:
                if (sensorValue < IR_OUT_RANGE_THRESHOLD && sensorValue > IR_IN_RANGE_THRESHOLD) {
                    boopStartTime = millis();
                    currentBoopState = BOOP_IN_PROGRESS;
                }
                isBoop = false;
                break;
            case BOOP_IN_PROGRESS:
                isInRange = true;
                if (sensorValue <= IR_IN_RANGE_THRESHOLD) {
                    boopSpeed = calculateBoopSpeed();
                    if (boopSpeed > 0.0) {
                        isBoop = true;
                    }
                    currentBoopState = IDLE;
                } else if (sensorValue >= IR_OUT_RANGE_THRESHOLD) {
                    currentBoopState = IDLE;
                }
                break;
        }
    }
};
