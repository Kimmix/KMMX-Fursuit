#pragma once
#include <Arduino.h>
#include "config.h"

enum BoopState {
    IDLE,
    BOOP_IN_PROGRESS,
    BOOP_CONTINUOUS,
    ANGRY
};

class Boop {
   private:
    BoopState currentBoopState = IDLE;
    unsigned long boopStartTime = 0;

    float calculateBoopSpeed();

   public:
    void getBoop(uint16_t& sensorValue, bool& isInRange, bool& isBoop, float& boopSpeed, bool& isContinuous, bool& isAngry);
};
