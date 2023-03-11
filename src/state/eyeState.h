#include <Arduino.h>
#include "Icons.h"
#include "draw.h"

class EyeState {
public:
    enum State {
        IDLE,
        BOOP,
    };

    EyeState(): currentState(IDLE) {}

    void update() {
        switch (currentState) {
        case IDLE:
            drawEye(eyeDefault);
            const uint8_t* eyeBMP;
            if (millis() >= nextBlink) {
                nextBlink = millis() + (1000 * random(4, 12));;
                drawEye(eyeBlink5);
                Serial.print("Blink \n");
            }
            break;
        case BOOP:
            arrowFace();
            break;
        default:
            break;
        }
    }

    void setIdle() {
        currentState = IDLE;
    }
    void setBoop() {
        currentState = BOOP;
    }

private:
    State currentState;
    volatile unsigned long nextBlink = 0, nextBoop = 0;

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
};