#include "RenderFunction/googlyEye.h"
#include "Bitmaps/eyeBitmap.h"
#include "Bitmaps/eyeBlink.h"
#include "Bitmaps/eyeSmile.h"
enum class EyeStateEnum { IDLE,
                          BLINK,
                          BOOP,
                          GOOGLY,
                          OEYE,
                          SMILE };
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
            case EyeStateEnum::SMILE:
                smileFace();
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
        isTransition = true;
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
    bool isTransition = false;
    const uint8_t* eyeFrame = eyeDefault;

    unsigned long
        nextBlink,
        blinkInterval,
        nextBoop,
        resetBoop,
        nextSmile;

    void movingEye() {
        float zAcc = event.acceleration.z;
        const float leftThreshold = 3.00, rightThreshold = -3.00,
                    leftMaxThreshold = 6.00, rightMaxThreshold = -6.00;
        if (zAcc > leftThreshold) {
            int level = mapFloat(zAcc, leftThreshold, leftMaxThreshold, 0, 19);
            display->drawEye(eyeFrame, eyeDown);
        } else if (zAcc < rightThreshold) {
            int level = mapFloat(zAcc, rightThreshold, rightMaxThreshold, 0, 19);
            display->drawEye(eyeDown, eyeFrame);
        } else {
            display->drawEye(eyeFrame);
        }
    }

    short defaultAnimationIndex = 0;
    const uint8_t* defaultAnimation[3] = {eyeDefault, eyeUp, eyeDown};
    void changeDefaultFace() {
        if ((esp_random() % 10) <= 3) {
            defaultAnimationIndex = (esp_random() % 2) + 1;
        } else {
            defaultAnimationIndex = 0;
        }
    }

    void idleFace() {
        // display->drawEye(eyeFrame);
        movingEye();
        if (millis() >= nextBlink) {
            nextBlink = millis() + (1000 * (esp_random() % 20));
            currentState = EyeStateEnum::BLINK;
        }
    }

    const uint8_t* blinkAnimation[15] = {
        eyeBlink1,
        eyeBlink2,
        eyeBlink3,
        eyeBlink4,
        eyeBlink5,
        eyeBlink6,
        eyeBlink7,
        eyeBlink8,
        eyeBlink9,
        eyeBlink10,
        eyeBlink11,
        eyeBlink12,
        eyeBlink13,
        eyeBlink14,
        eyeBlink15};
    const short blinkAnimationLength = 15;
    short blinkStep, currentBlinkFrameIndex;
    void blink() {
        if (millis() >= blinkInterval) {
            if (blinkStep < blinkAnimationLength - 1) {
                blinkStep++;
                currentBlinkFrameIndex++;
                blinkInterval = millis() + 7;
            } else if (blinkStep >= blinkAnimationLength - 1 && blinkStep < (blinkAnimationLength * 2) - 1) {
                blinkStep++;
                currentBlinkFrameIndex--;
                blinkInterval = millis() + 4;
            }
            if (blinkStep == (blinkAnimationLength * 2) - 1) {
                blinkStep = 0;
                currentBlinkFrameIndex = 0;
                changeDefaultFace();
                currentState = EyeStateEnum::IDLE;  // Blink complete, reset to idle
            }
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

    const uint8_t* smileAnimation[20] = {
        eyeSmile1,
        eyeSmile2,
        eyeSmile3,
        eyeSmile4,
        eyeSmile5,
        eyeSmile6,
        eyeSmile7,
        eyeSmile8,
        eyeSmile9,
        eyeSmile10,
        eyeSmile11,
        eyeSmile12,
        eyeSmile13,
        eyeSmile14,
        eyeSmile15,
        eyeSmile16,
        eyeSmile17,
        eyeSmile18,
        eyeSmile19,
        eyeSmile20};
    const short smileLength = 20;
    short smileIndex = 0;
    void smileFace() {
        if (isTransition) {
            display->drawEye(smileAnimation[smileIndex]);
            if (millis() >= nextSmile) {
                nextSmile = millis() + 7;
                smileIndex++;
            }
            if (smileIndex >= (smileLength - 1)) {
                isTransition = false;
                smileIndex = 0;
            }
        }
        display->drawEye(smileAnimation[smileLength - 1]);
    }

    void renderGooglyEye() {
        display->drawEye(eyeGoogly);
        googlyEye.renderEye(event.acceleration.x, event.acceleration.y);
        display->drawEyePupil(eyePupil, googlyEye.x, googlyEye.y);
    }
};