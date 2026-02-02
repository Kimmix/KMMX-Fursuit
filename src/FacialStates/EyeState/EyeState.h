#pragma once

#include <Adafruit_Sensor.h>
#include "Devices/LEDMatrixDisplay/Hub75DMA.h"
#include "Bitmaps/Bitmaps.h"
#include "Utils/Utils.h"
#include "Renderer/GooglyEye.h"
#include "Types/SensorData.h"

enum class EyeStateEnum { IDLE,
                          BLINK,
                          BOOP,
                          GOOGLY,
                          OEYE,
                          HEART,
                          SMILE,
                          ANGRY,
                          SLEEP,
                          DETRANSITION };

class EyeState {
   public:
    EyeState(Hub75DMA* display);
    void update();
    void setState(EyeStateEnum newState);
    void savePrevState(EyeStateEnum newState);
    void playPrevState();
    EyeStateEnum getState() const;
    void setSensorData(const SensorData& data);

   private:
    Hub75DMA* display;
    SensorData sensorData;
    GooglyEye googlyEye;
    EyeStateEnum prevState, currentState = EyeStateEnum::IDLE;
    const uint8_t* eyeFrame = eyeDefault;
    bool isTransitioning = false;

    unsigned long resetBoop, blinkInterval, nextBlink, nextBoop, nextSmile, nextDown, nextAngry, nextSleep;

    uint8_t defaultAnimationIndex = 0;
    const uint8_t* defaultAnimation[3] = {eyeDefault, eyeUp20, eyeLookSharp5};

    const uint8_t* eyeDownAnimation[20] = {
        eyeDown1, eyeDown2, eyeDown3, eyeDown4, eyeDown5, eyeDown6, eyeDown7, eyeDown8, eyeDown9, eyeDown10,
        eyeDown11, eyeDown12, eyeDown13, eyeDown14, eyeDown15, eyeDown16, eyeDown17, eyeDown18, eyeDown19, eyeDown20};
    const uint8_t* eyeUpAnimation[20] = {
        eyeUp1, eyeUp2, eyeUp3, eyeUp4, eyeUp5, eyeUp6, eyeUp7, eyeUp8, eyeUp9, eyeUp10,
        eyeUp11, eyeUp12, eyeUp13, eyeUp14, eyeUp15, eyeUp16, eyeUp17, eyeUp18, eyeUp19, eyeUp20};

    const uint8_t* blinkAnimation[24] = {eyeBlink1, eyeBlink2, eyeBlink3, eyeBlink4, eyeBlink5, eyeBlink6, eyeBlink7, eyeBlink8, eyeBlink9, eyeBlink10, eyeBlink11, eyeBlink12, eyeBlink13, eyeBlink14, eyeBlink15, eyeBlink16, eyeBlink17, eyeBlink18, eyeBlink19, eyeBlink20, eyeBlink21, eyeBlink22, eyeBlink23, eyeBlink24};
    const uint8_t blinkAnimationLength = arrayLength(blinkAnimation);

    uint8_t currentBlinkFrameIndex = 0;
    bool blinkDirection = true;  // true = closing, false = opening
    uint8_t blinkType = 0;       // 0 = normal, 1 = quick, 2 = slow
    uint8_t blinkCount = 0;      // Track blinks for double blink
    bool shouldDoubleBlink = false;

    // Optimized timing tables [blinkType][isClosing]
    const uint8_t blinkTimings[3][2] = {
        {3, 5},  // Normal: ~36ms closing + ~60ms opening = ~96ms
        {2, 3},  // Quick: ~24ms closing + ~36ms opening = ~60ms
        {5, 8}   // Slow: ~60ms closing + ~96ms opening = ~156ms
    };

    const uint8_t* boopAnimation[2] = {eyeV1, eyeV2};
    uint8_t boopAnimationFrame = 0;

    const uint8_t* oFaceAnimation[3] = {eyeO1, eyeO2, eyeO3};
    uint8_t currentOFaceIndex = 0;

    const uint8_t* smileAnimation[20] = {
        eyeSmile1, eyeSmile2, eyeSmile3, eyeSmile4, eyeSmile5, eyeSmile6, eyeSmile7, eyeSmile8, eyeSmile9, eyeSmile10,
        eyeSmile11, eyeSmile12, eyeSmile13, eyeSmile14, eyeSmile15, eyeSmile16, eyeSmile17, eyeSmile18, eyeSmile19, eyeSmile20};
    const uint8_t smileLength = arrayLength(smileAnimation);
    uint8_t smileIndex = 0;

    const uint8_t* eyeAngryAnimation[20] = {
        eyeAngry1, eyeAngry2, eyeAngry3, eyeAngry4, eyeAngry5, eyeAngry6, eyeAngry7, eyeAngry8, eyeAngry9, eyeAngry10,
        eyeAngry11, eyeAngry12, eyeAngry13, eyeAngry14, eyeAngry15, eyeAngry16, eyeAngry17, eyeAngry18, eyeAngry19, eyeAngry20};
    const uint8_t angryLength = arrayLength(eyeAngryAnimation);
    uint8_t angryIndex = 0;

    const uint8_t* eyeSleepAnimation[20] = {
        eyeSleep1, eyeSleep2, eyeSleep3, eyeSleep4, eyeSleep5, eyeSleep6, eyeSleep7, eyeSleep8, eyeSleep9, eyeSleep10,
        eyeSleep11, eyeSleep12, eyeSleep13, eyeSleep14, eyeSleep15, eyeSleep16, eyeSleep17, eyeSleep18, eyeSleep19, eyeSleep20};
    const uint8_t sleepLength = arrayLength(eyeSleepAnimation);
    uint8_t sleepIndex = 0, sleepRand = 0;
    unsigned long startSleepTime;
    int calculateSleepIndex(int currentIndex);

    void changeDefaultFace();
    void movingEye();
    void idleFace();
    void blink();
    void arrowFace();
    void oFace();
    void smileFace();
    void angryFace();
    void sleepFace();
    void resetSleepFace();
    void detransition();
    void renderGooglyEye();
};
