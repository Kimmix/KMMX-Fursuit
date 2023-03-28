#include <Arduino.h>
#include "../RenderFuction/googlyEye.h"


class EyeState {
public:
    EyeState(DisplayController* displayPtr = nullptr):
        display(displayPtr),
        // bmi160(bmi160Ptr),
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
                nextBlink = millis() + (1000 * random(1, 12));
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
                currentState = IDLE;
            }
            break;
        case GOOGLY:
            googlyEye();
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
    void setGoogly() {
        currentState = GOOGLY;
    }

private:
    DisplayController* display;
    GooglyEye eye;

    enum State {
        IDLE,
        BLINK,
        BOOP,
        GOOGLY,
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

    const uint8_t* blinkAnimation[4] = {
        eyeBlink1, eyeBlink2, eyeBlink3, eyeBlink3
    };
    int blinkAnimationLength = 4;
    int blinkStep, currentBlinkFrameIndex;
    void blink() {
        if (millis() >= blinkInterval) {
            if (blinkStep < blinkAnimationLength - 1) {
                blinkStep++;
                currentBlinkFrameIndex++;
            }
            else if (blinkStep >= blinkAnimationLength - 1 && blinkStep < (blinkAnimationLength * 2) - 1) {
                blinkStep++;
                currentBlinkFrameIndex--;
            }
            if (blinkStep == (blinkAnimationLength * 2) - 1) {
                blinkStep = 0;
                currentBlinkFrameIndex = 0;
                currentState = IDLE; // Blink complete, reset to idle
            }
            blinkInterval = millis() + 50;
        }
        display->drawEye(blinkAnimation[currentBlinkFrameIndex]);
    }

    const int ACC_FILTER = 1;
    void googlyEye() {
        // // Get accelerometer data
        // int16_t accel[6] = { 0 };
        // bmi160->getAccelGyroData(accel);
        // float ax = static_cast<float>(accel[0]) * 3.14 / 180.0;
        // float ay = static_cast<float>(accel[1]) * 3.14 / 180.0;
        // float az = static_cast<float>(accel[2]) * 3.14 / 180.0;

        // // Orient the sensor directions to the display directions
        // float eye_ax = -az;
        // float eye_ay = -ax;

        // // Apply accelerometer filter
        // if (eye_ax < ACC_FILTER) {
        //     eye_ax = 0;
        // }
        // if (eye_ay < ACC_FILTER) {
        //     eye_ay = 0;
        // }

        // // Update eye position and draw on display
        // eye.update(eye_ax, eye_ay);
        // display->drawEye(eyeGoogly);
        // display->drawEyePupil(eyePupil, static_cast<int>(eye.x), static_cast<int>(eye.y));
    }
};