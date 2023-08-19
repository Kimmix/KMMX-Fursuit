#include "RenderFunction/googlyEye.h"
#include "Bitmaps/eyeBitmap.h"

class EyeState {
   public:
    EyeState(LEDMatrixDisplay* display) : display(display) {}

    void update() {
        switch (currentState) {
            case EyeStateEnum::IDLE:
                idleFace();
                break;
            case EyeStateEnum::BLINK:
                blink();
                break;
            case EyeStateEnum::BOOP:
                arrowFace();
                if (millis() - resetBoop >= 1500) {
                    currentState = prevState;
                }
                break;
            case EyeStateEnum::GOOGLY:
                renderGooglyEye();
                break;
            case EyeStateEnum::OEYE:
                oFace();
                break;
            default:
                break;
        }
    }

    void setState(EyeStateEnum newState) {
        savePrevState(currentState);
        if (newState == EyeStateEnum::BOOP) {
            resetBoop = millis();
        }
        currentState = newState;
    }

    void savePrevState(EyeStateEnum newState) {
        if (newState == EyeStateEnum::BOOP) {
            return;
        }
        prevState = newState;
    }

    EyeStateEnum getState() const {
        return currentState;
    }

    void getListEvent(const sensors_event_t& eventData) {
        event = eventData;
    }

   private:
    LEDMatrixDisplay* display;
    sensors_event_t event;
    GooglyEye googlyEye;
    EyeStateEnum prevState, currentState = EyeStateEnum::IDLE;

    unsigned long
        nextBlink,
        blinkInterval,
        nextBoop,
        resetBoop;

    short defaultFaceIndex = 0;
    const uint8_t* defaultAnimation[3] = {eyeDefault, eyeUp, eyeDown};
    void changeDefaultFace() {
        if ((esp_random() % 10) <= 3) {
            defaultFaceIndex = (esp_random() % 2) + 1;
        } else {
            defaultFaceIndex = 0;
        }
    }

    void idleFace() {
        display->drawEye(defaultAnimation[defaultFaceIndex]);
        if (millis() >= nextBlink) {
            nextBlink = millis() + (1000 * (esp_random() % 20));
            currentState = EyeStateEnum::BLINK;
        }
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
                currentState = EyeStateEnum::IDLE;  // Blink complete, reset to idle
            }
            blinkInterval = millis() + 70;
        }
        display->drawEye(blinkAnimation[currentBlinkFrameIndex]);
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

    void renderGooglyEye() {
        display->drawEye(eyeGoogly);
        googlyEye.renderEye(event.acceleration.x, event.acceleration.y);
        display->drawEyePupil(eyePupil, googlyEye.x, googlyEye.y);
    }
};