#include <Arduino.h>

class MouthState {
public:
    MouthState(DisplayController* displayPtr = nullptr): display(displayPtr),
        currentState(IDLE),
        nextBlink(0),
        blinkInterval(0),
        nextBoop(0),
        resetBoop_(0)
    {}

    void update() {
        Serial.print(currentState);
        Serial.print("\n");
        switch (currentState) {
        case IDLE:
            display->drawMouth(mouthDefault);
            break;
        case BOOP:
            display->drawMouth(mouthOpen);
            currentState = IDLE;
            break;
        case TALKING:
            break;
        default:
            break;
        }
    }

    void setIdle() {
        currentState = IDLE;
    }
    void setBlink() {
        currentState = TALKING;
    }
    void setBoop() {
        currentState = BOOP;
    }

private:
    DisplayController* display;

    enum State {
        IDLE,
        BOOP,
        TALKING,
    };

    State currentState;
    unsigned long
        nextBlink,
        blinkInterval,
        nextBoop,
        resetBoop_;

};