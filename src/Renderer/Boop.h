#pragma once
#include <Arduino.h>
#include "config.h"

enum class BoopEvent {
    IDLE,
    APPROACHING,
    COMPLETED,
    HELD,
    TOO_CLOSE,
    LOCKED_OUT,
    RELEASED,
    INCOMPLETE_RELEASE
};

struct BoopResult {
    BoopEvent event = BoopEvent::IDLE;
    float speed = 0.0f;
};

class Boop {
   private:
    enum class State { IDLE, APPROACHING, HELD, TOO_CLOSE };

    State state = State::IDLE;
    unsigned long approachStartedAt = 0;
    unsigned long tooCloseStartedAt = 0;

    static constexpr unsigned long disableAfterTooCloseMs = 10000;

    static float speedFor(unsigned long elapsed);

   public:
    BoopResult update(uint16_t proximity, unsigned long now = millis());
};
