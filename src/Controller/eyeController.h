#include "../RenderFunction/googlyEye.h"
#include "Bitmaps/eyeBitmap.h"

class EyeState {
   public:
    EyeState(DisplayController* displayPtr = nullptr, LIS3DH* lisPtr = nullptr) : display(displayPtr),
                                                                                  lis(lisPtr),
                                                                                  currentState(GOOGLY),
                                                                                  nextBlink(0),
                                                                                  blinkInterval(0),
                                                                                  nextFace(0),
                                                                                  nextBoop(0),
                                                                                  resetBoop_(0),
                                                                                  boopAnimationFrame(0),
                                                                                  currentOFaceIndex(0),
                                                                                  blinkStep(0),
                                                                                  currentBlinkFrameIndex(0) {}

    void update() {
        // Serial.print(currentState);
        // Serial.print("\n");
        switch (currentState) {
            case IDLE:
                display->drawEye(defaultAnimation[defaultFaceIndex]);
                if (millis() >= nextBlink) {
                    nextBlink = millis() + (500 * (esp_random() % 25));
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
    }
    void setGoogly() {
        currentState = GOOGLY;
    }

   private:
    DisplayController* display;
    LIS3DH* lis;
    GooglyEye googlyEye;

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
        nextFace,
        nextBoop,
        resetBoop_;

    short defaultFaceIndex = 0;
    const uint8_t* defaultAnimation[3] = {eyeDefault, eyeUp, eyeDown};
    void changeDefaultFace() {
        if ((esp_random() % 10) <= 3) {
            defaultFaceIndex = (esp_random() % 2) + 1;
        } else {
            defaultFaceIndex = 0;
        }
    }

    const uint8_t* boopAnimation[2] = {eyeV1, eyeV2};
    short boopAnimationFrame;
    void arrowFace() {
        if (millis() > nextBoop) {
            nextBoop = millis() + 500;
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

    const uint8_t* blinkAnimation[3] = {
        eyeBlink1, eyeBlink2, eyeBlink3};
    int blinkAnimationLength = 3;
    int blinkStep, currentBlinkFrameIndex;
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
            blinkInterval = millis() + 40;
        }
        display->drawEye(blinkAnimation[currentBlinkFrameIndex]);
    }

    const float ACC_FILTER = 1;
    void renderGooglyEye() {
        float x, y, z;
        lis->readAccelG(x, y, z);
        // Serial.print("X:"); Serial.print(ax);
        // Serial.print(",Y:"); Serial.print(ay);
        // Serial.print(",Z:"); Serial.println(az);

        // Orient the sensor directions to the display directions
        float eye_ax = -y;
        float eye_ay = -x;

        // Update googlyEye position and draw on display
        display->drawEye(eyeGoogly);
        googlyEye.update(eye_ax, eye_ay);
        display->drawEyePupil(eyePupil, googlyEye.x, googlyEye.y);
    }
};