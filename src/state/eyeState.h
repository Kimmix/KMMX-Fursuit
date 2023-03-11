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
        // Serial.print(currentState);
        // Serial.print("\n");
        switch (currentState) {
        case IDLE:
            drawEye(eyeDefault);
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
    State currentState;
    volatile unsigned long
        nextBlink = 0,
        blinkInterval = 0,
        nextBoop = 0,
        resetBoop_;

    const uint8_t* boopAnimation[2] = { eyeV1, eyeV2 };
    short boopAnimationFrame = 0;
    void arrowFace() {
        if (millis() > nextBoop) {
            nextBoop = millis() + 500;
            boopAnimationFrame ^= 1;
        }
        drawEye(boopAnimation[boopAnimationFrame]);
    }

    const uint8_t* oFaceAnimation[3] = { eyeO1, eyeO2, eyeO3 };
    short oFaceAnimationFrame = 0;
    void oFace() {
        if (millis() >= nextBoop) {
            nextBoop = millis() + 200;
            oFaceAnimationFrame = random(0, 2);
        }
        drawEye(oFaceAnimation[oFaceAnimationFrame]);
    }

    const uint8_t* blinkAnimation[8] = {
        eyeDefault, eyeBlink1, eyeBlink2, eyeBlink3,
        eyeBlink4,  eyeBlink5, eyeBlink6, eyeBlink7,
    };
    int blinkStep = 0, blinkAnimationStep = 0;
    void blink() {
        if (millis() >= blinkInterval) {
            if (blinkStep < 7) {
                blinkStep++;
                blinkAnimationStep++;
            }
            else if (blinkStep >= 7 && blinkStep < 15) {
                blinkStep++;
                blinkAnimationStep--;
            }
            if (blinkStep == 15) {
                blinkStep = 0;
                blinkAnimationStep = 0;
                currentState = IDLE; // Blink complete, reset to idle
            }
            blinkInterval = millis() + 50;
        }
        drawEye(blinkAnimation[blinkAnimationStep]);
    }
};