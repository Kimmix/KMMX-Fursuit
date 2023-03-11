#include <Arduino.h>
#include "Icons.h"
#include "draw.h"

class EyeState {
public:
    enum State {
        IDLE,
        BLINK,
        BOOP,
    };

    EyeState(): currentState(IDLE) {}

    void update() {
        switch (currentState) {
        case IDLE:
            drawEye(eyeDefault);
            break;
        case BLINK:
            drawEye(eyeBlink4);
            break;
        case BOOP:
            drawEye(eyeV1);
            break;
        default:
            break;
        }
    }

    void setIdle() {
        currentState = IDLE;
    }
    void setBlink() {
        currentState = BLINK;
    }
    void setBoop() {
        currentState = BOOP;
    }

private:
    State currentState;

};