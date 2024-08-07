#include "RenderFunction/GooglyEye.h"
#include "Bitmaps/eye/eyeBitmap.h"
#include "Bitmaps/eye/eyeBlink.h"
#include "Bitmaps/eye/eyeUp.h"
#include "Bitmaps/eye/eyeDown.h"
#include "Bitmaps/eye/eyeAngry.h"
#include "Bitmaps/eye/eyeSleep.h"
#include "Bitmaps/eye/eyeLookSharp.h"
#include "Bitmaps/eye/eyeSmile.h"
enum class EyeStateEnum { IDLE,
                          BLINK,
                          BOOP,
                          GOOGLY,
                          OEYE,
                          HEART,
                          SMILE,
                          ANGRY,
                          SLEEP,
                          DETRANSITION,
};
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
            case EyeStateEnum::HEART:
                display->drawEye(eyeHeart);
                break;
            case EyeStateEnum::SMILE:
                smileFace();
                break;
            case EyeStateEnum::ANGRY:
                angryFace();
                if (millis() - resetBoop >= 2000) {
                    currentState = prevState;
                }
                break;
            case EyeStateEnum::SLEEP:
                sleepFace();
                break;
            case EyeStateEnum::DETRANSITION:
                detransition();
                break;
            default:
                break;
        }
    }

    void setState(EyeStateEnum newState) {
        savePrevState(currentState);
        if (newState == EyeStateEnum::BOOP || newState == EyeStateEnum::ANGRY) {
            resetBoop = millis();
        }
        if (newState == EyeStateEnum::SLEEP) {
            resetSleepFace();
        }
        if (currentState != newState) {
            isTransitioning = true;
            currentState = newState;
        }
    }

    void savePrevState(EyeStateEnum newState) {
        if (newState == EyeStateEnum::BOOP || newState == EyeStateEnum::ANGRY || newState == EyeStateEnum::SLEEP) {
            return;
        }
        prevState = newState;
    }

    void playPrevState() {
        setState(prevState);
        resetSleepFace();
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
    const uint8_t* eyeFrame = eyeDefault;
    bool isTransitioning = false;

    unsigned long
        resetBoop,
        blinkInterval,
        nextBlink,
        nextBoop,
        nextSmile,
        nextDown,
        nextAngry,
        nextSleep;

    short defaultAnimationIndex = 0;
    const uint8_t* defaultAnimation[3] = {eyeDefault, eyeUp20, eyeLookSharp5};
    void changeDefaultFace() {
        if ((esp_random() % 10) <= 3) {
            defaultAnimationIndex = (esp_random() % 2) + 1;
        } else {
            defaultAnimationIndex = 0;
        }
        Serial.print(F("Changing faces to:"));
        Serial.println(defaultAnimationIndex);
        eyeFrame = defaultAnimation[defaultAnimationIndex];
    }

    const uint8_t* eyeDownAnimation[20] = {eyeDown1, eyeDown2, eyeDown3, eyeDown4, eyeDown5, eyeDown6, eyeDown7, eyeDown8, eyeDown9, eyeDown10, eyeDown11, eyeDown12, eyeDown13, eyeDown14, eyeDown15, eyeDown16, eyeDown17, eyeDown18, eyeDown19, eyeDown20};
    const uint8_t* eyeUpAnimation[20] = {eyeUp1, eyeUp2, eyeUp3, eyeUp4, eyeUp5, eyeUp6, eyeUp7, eyeUp8, eyeUp9, eyeUp10, eyeUp11, eyeUp12, eyeUp13, eyeUp14, eyeUp15, eyeUp16, eyeUp17, eyeUp18, eyeUp19, eyeUp20};
    void movingEye() {
        float zAcc = event.acceleration.z;
        const float leftThreshold = 3.00, rightThreshold = -3.00,
                    leftMaxThreshold = 6.00, rightMaxThreshold = -6.00;
        if (zAcc > leftThreshold) {
            int level = mapFloat(zAcc, leftThreshold, leftMaxThreshold, 0, 19);
            display->drawEye(eyeUpAnimation[level], eyeDownAnimation[level]);
        } else if (zAcc < rightThreshold) {
            int level = mapFloat(zAcc, rightThreshold, rightMaxThreshold, 0, 19);
            display->drawEye(eyeDownAnimation[level], eyeUpAnimation[level]);
        } else {
            display->drawEye(eyeFrame);
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

    const uint8_t* blinkAnimation[15] = {eyeBlink1, eyeBlink2, eyeBlink3, eyeBlink4, eyeBlink5, eyeBlink6, eyeBlink7, eyeBlink8, eyeBlink9, eyeBlink10, eyeBlink11, eyeBlink12, eyeBlink13, eyeBlink14, eyeBlink15};
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

    const uint8_t* smileAnimation[20] = {eyeSmile1, eyeSmile2, eyeSmile3, eyeSmile4, eyeSmile5, eyeSmile6, eyeSmile7, eyeSmile8, eyeSmile9, eyeSmile10, eyeSmile11, eyeSmile12, eyeSmile13, eyeSmile14, eyeSmile15, eyeSmile16, eyeSmile17, eyeSmile18, eyeSmile19, eyeSmile20};
    const short smileLength = 20;
    short smileIndex = 0;
    void smileFace() {
        if (isTransitioning) {
            display->drawEye(smileAnimation[smileIndex]);
            if (millis() >= nextSmile) {
                nextSmile = millis() + 14;
                smileIndex++;
                if (smileIndex == smileLength) {
                    smileIndex = 0;
                    isTransitioning = false;
                }
            }
        } else {
            display->drawEye(smileAnimation[smileLength - 1]);
        }
    }

    const uint8_t* eyeangryAnimation[20] = {eyeAngry1, eyeAngry2, eyeAngry3, eyeAngry4, eyeAngry5, eyeAngry6, eyeAngry7, eyeAngry8, eyeAngry9, eyeAngry10, eyeAngry11, eyeAngry12, eyeAngry13, eyeAngry14, eyeAngry15, eyeAngry16, eyeAngry17, eyeAngry18, eyeAngry19, eyeAngry20};
    const short angryLength = 20;
    short angryIndex = 0;
    void angryFace() {
        if (isTransitioning) {
            display->drawEye(eyeangryAnimation[angryIndex]);
            if (millis() >= nextAngry) {
                nextAngry = millis() + 5;
                angryIndex++;
                if (angryIndex == angryLength) {
                    angryIndex = 0;
                    isTransitioning = false;
                }
            }
        } else {
            display->drawEye(eyeangryAnimation[angryLength - 1]);
        }
    }

    const uint8_t* eyesleepAnimation[20] = {eyeSleep1, eyeSleep2, eyeSleep3, eyeSleep4, eyeSleep5, eyeSleep6, eyeSleep7, eyeSleep8, eyeSleep9, eyeSleep10, eyeSleep11, eyeSleep12, eyeSleep13, eyeSleep14, eyeSleep15, eyeSleep16, eyeSleep17, eyeSleep18, eyeSleep19, eyeSleep20};
    const int sleepLength = 20;
    int sleepIndex = 0, sleepRand = 0;
    unsigned long startSleepTime = millis();
    // Function to calculate the next sleep index
    int calculateSleepIndex(int currentIndex, int randomValue) {
        int minIndex = min(pow((millis() - startSleepTime) / 5000, 2), 19.0);
        if (randomValue > 7) {
            return min(currentIndex + 1, sleepLength - 1);
        } else if (randomValue < 5) {
            return max(currentIndex - 1, minIndex);
        }
        return currentIndex;
    }

    // Updated sleepFace function
    void sleepFace() {
        if (millis() >= nextSleep) {
            if (sleepIndex < sleepLength - 1) {
                nextSleep = millis() + 300;
                sleepRand = esp_random() % 10;
                sleepIndex = calculateSleepIndex(sleepIndex, sleepRand);
            }
        }
        display->drawEye(eyesleepAnimation[sleepIndex]);
    }

    // Function to reset sleep face
    void resetSleepFace() {
        sleepIndex = 0;
        startSleepTime = millis();
    }
    // short smileIndex2 = smileLength - 1;
    // void smileDeTransition() {
    //     bool isDeTransitioning = true;
    //     while (isDeTransitioning) {
    //         Serial.println(smileIndex2);
    //         display->drawEye(smileAnimation[smileIndex2]);
    //         if (millis() >= nextSmile) {
    //             nextSmile = millis() + 14;
    //             smileIndex2--;
    //             if (smileIndex2 == 0) {
    //                 smileIndex2 = smileLength - 1;
    //                 isDeTransitioning = false;
    //             }
    //         }
    //     }
    // }

    void detransition() {
    }

    void renderGooglyEye() {
        display->drawEye(eyeGoogly);
        googlyEye.renderEye(event.acceleration.z, event.acceleration.x);
        display->drawEyePupil(eyePupil, googlyEye.x, googlyEye.y);
    }
};