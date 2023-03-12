#include <Arduino.h>

class EyeState {
public:
    EyeState(DisplayController* displayPtr = nullptr): display(displayPtr),
        currentState(IDLE),
        nextBlink(0),
        blinkInterval(0),
        nextBoop(0),
        resetBoop_(0),
        boopAnimationFrame(0),
        currentOFaceIndex(0),
        blinkStep(0),
        currentBlinkFrameIndex(0)
    {}

    void update() {
        // Serial.print(currentState);
        // Serial.print("\n");
        switch (currentState) {
        case IDLE:
            display->drawEye(eyeDefault);
            if (millis() >= nextBlink) {
                nextBlink = millis() + (1000 * random(4, 12));
                currentState = BLINK;
            }
            break;
        case BLINK:
            blink();
            break;
        case BOOP:
            arrowFace();
            if (millis() - resetBoop_ >= 2000) {
                resetBoop_ = millis();
                Serial.print("Reseting.. \n");
                currentState = IDLE;
            }
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
    DisplayController* display;

    enum State {
        IDLE,
        BLINK,
        BOOP,
    };

    State currentState;
    unsigned long
        nextBlink,
        blinkInterval,
        nextBoop,
        resetBoop_;

    const uint8_t* boopAnimation[2] = { eyeV1, eyeV2 };
    short boopAnimationFrame;
    void arrowFace() {
        if (millis() > nextBoop) {
            nextBoop = millis() + 500;
            boopAnimationFrame ^= 1;
        }
        display->drawEye(boopAnimation[boopAnimationFrame]);
    }

    const uint8_t* oFaceAnimation[3] = { eyeO1, eyeO2, eyeO3 };
    short currentOFaceIndex;
    void oFace() {
        if (millis() >= nextBoop) {
            nextBoop = millis() + 200;
            currentOFaceIndex = random(0, 3);
        }
        display->drawEye(oFaceAnimation[currentOFaceIndex]);
    }

    const uint8_t* blinkAnimation[8] = {
        eyeDefault, eyeBlink1, eyeBlink2, eyeBlink3,
        eyeBlink4,  eyeBlink5, eyeBlink6, eyeBlink7,
    };
    int blinkStep, currentBlinkFrameIndex;
    void blink() {
        if (millis() >= blinkInterval) {
            if (blinkStep < 7) {
                blinkStep++;
                currentBlinkFrameIndex++;
            }
            else if (blinkStep >= 7 && blinkStep < 15) {
                blinkStep++;
                currentBlinkFrameIndex--;
            }
            if (blinkStep == 15) {
                blinkStep = 0;
                currentBlinkFrameIndex = 0;
                currentState = IDLE; // Blink complete, reset to idle
            }
            blinkInterval = millis() + 50;
        }
        display->drawEye(blinkAnimation[currentBlinkFrameIndex]);
    }
};