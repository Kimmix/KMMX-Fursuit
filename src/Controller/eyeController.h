#include "../RenderFunction/googlyEye.h"
#include "Bitmaps/eyeBitmap.h"

class EyeState {
   public:
    EyeState(LEDMatrixDisplay* displayPtr = nullptr) : display(displayPtr) {}

    void update() {
        // Serial.print(currentState);
        // Serial.print("\n");
        switch (currentState) {
            case IDLE:
                idleFace();
                break;
            case BLINK:
                blink();
                break;
            case BOOP:
                boopFace();
                break;
            case GOOGLY:
                renderGooglyEye();
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
        resetBoop = millis();
    }
    void setGoogly() {
        currentState = GOOGLY;
    }

   private:
    LEDMatrixDisplay* display;
    GooglyEye googlyEye;

    enum State {
        IDLE,
        BLINK,
        BOOP,
        GOOGLY,
    };
    State currentState = IDLE;

    unsigned long
        nextBlink,
        blinkInterval,
        nextBoop,
        resetBoop;

    void idleFace() {
        display->drawEye(defaultAnimation[defaultFaceIndex]);
        if (millis() >= nextBlink) {
            nextBlink = millis() + (1000 * (esp_random() % 25) + 1);
            currentState = BLINK;
        }
    }

    short defaultFaceIndex = 0;
    const uint8_t* defaultAnimation[3] = {eyeDefault, eyeUp, eyeDown};
    void changeDefaultFace() {
        if ((esp_random() % 10) <= 3) {
            defaultFaceIndex = (esp_random() % 2) + 1;
        } else {
            defaultFaceIndex = 0;
        }
    }

    void boopFace() {
        arrowFace();
        if (millis() - resetBoop >= 1000) {
            currentState = IDLE;
        }
    }

    const uint8_t* boopAnimation[2] = {eyeV1, eyeV2};
    short boopAnimationFrame;
    void arrowFace() {
        if (millis() > nextBoop) {
            nextBoop = millis() + 300;
            boopAnimationFrame ^= 1;
        }
        display->drawEye(boopAnimation[boopAnimationFrame]);
    }

    const uint8_t* oFaceAnimation[3] = {eyeO1, eyeO2, eyeO3};
    short currentOFaceIndex;
    void oFace() {
        if (millis() >= nextBoop) {
            nextBoop = millis() + 200;
            currentOFaceIndex = esp_random() % 3;
        }
        display->drawEye(oFaceAnimation[currentOFaceIndex]);
    }

    const uint8_t* blinkAnimation[3] = {eyeBlink1, eyeBlink2, eyeBlink3};
    const short blinkAnimationLength = 3;
    short blinkStep, currentBlinkFrameIndex;
    void blink() {
        if (millis() >= blinkInterval) {
            if (blinkStep < blinkAnimationLength - 1) {
                blinkStep++;
                currentBlinkFrameIndex++;
            } else if (blinkStep >= blinkAnimationLength - 1 && blinkStep < (blinkAnimationLength * 2) - 1) {
                blinkStep++;
                currentBlinkFrameIndex--;
            }
            if (blinkStep == (blinkAnimationLength * 2) - 1) {
                blinkStep = 0;
                currentBlinkFrameIndex = 0;
                changeDefaultFace();
                currentState = IDLE;  // Blink complete, reset to idle
            }
            blinkInterval = millis() + 70;
        }
        display->drawEye(blinkAnimation[currentBlinkFrameIndex]);
    }

    void renderGooglyEye() {
        display->drawEye(eyeGoogly);
        googlyEye.renderEye();
        display->drawEyePupil(eyePupil, googlyEye.x, googlyEye.y);
    }
};