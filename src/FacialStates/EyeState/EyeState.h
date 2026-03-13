#pragma once

#include <Adafruit_Sensor.h>
#include "Devices/LEDMatrixDisplay/Hub75DMA.h"
#include "Bitmaps/Bitmaps.h"
#include "Utils/Utils.h"
#include "Renderer/GooglyEye.h"
#include "Renderer/TimeBasedAnimation.h"
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
    bool isTransitioning = false;

    unsigned long resetBoop, blinkInterval, nextBlink, nextBoop, nextSmile, nextDown, nextAngry, nextSleep;
    unsigned long nextIdleAction;  // Timer for micro-movements and eye darts

    // Idle micro-movement frames for variety
    const uint8_t* idleLookFrames[6] = {eyeDefault, eyeUp5, eyeUp10, eyeLookSharp5, eyeLookSharp10, eyeGiggle8};
    uint8_t currentIdleFrame = 0;

    const uint8_t* eyeDownAnimation[20] = {
        eyeDown1, eyeDown2, eyeDown3, eyeDown4, eyeDown5, eyeDown6, eyeDown7, eyeDown8, eyeDown9, eyeDown10,
        eyeDown11, eyeDown12, eyeDown13, eyeDown14, eyeDown15, eyeDown16, eyeDown17, eyeDown18, eyeDown19, eyeDown20};
    const uint8_t* eyeUpAnimation[20] = {
        eyeUp1, eyeUp2, eyeUp3, eyeUp4, eyeUp5, eyeUp6, eyeUp7, eyeUp8, eyeUp9, eyeUp10,
        eyeUp11, eyeUp12, eyeUp13, eyeUp14, eyeUp15, eyeUp16, eyeUp17, eyeUp18, eyeUp19, eyeUp20};

    const uint8_t* blinkAnimation[24] = {eyeBlink1, eyeBlink2, eyeBlink3, eyeBlink4, eyeBlink5, eyeBlink6, eyeBlink7, eyeBlink8, eyeBlink9, eyeBlink10, eyeBlink11, eyeBlink12, eyeBlink13, eyeBlink14, eyeBlink15, eyeBlink16, eyeBlink17, eyeBlink18, eyeBlink19, eyeBlink20, eyeBlink21, eyeBlink22, eyeBlink23, eyeBlink24};
    const uint8_t blinkAnimationLength = arrayLength(blinkAnimation);

    TimeBasedAnimState blinkAnim;
    uint8_t blinkCount = 0;      // Track blinks for double blink
    bool shouldDoubleBlink = false;

    const uint8_t* boopAnimation[48] = {
        eyeArrow1, eyeArrow2, eyeArrow3, eyeArrow4, eyeArrow5, eyeArrow6, eyeArrow7, eyeArrow8, eyeArrow9, eyeArrow10,
        eyeArrow11, eyeArrow12, eyeArrow13, eyeArrow14, eyeArrow15, eyeArrow16, eyeArrow17, eyeArrow18, eyeArrow19, eyeArrow20,
        eyeArrow21, eyeArrow22, eyeArrow23, eyeArrow24, eyeArrow25, eyeArrow26, eyeArrow27, eyeArrow28, eyeArrow29, eyeArrow30,
        eyeArrow31, eyeArrow32, eyeArrow33, eyeArrow34, eyeArrow35, eyeArrow36, eyeArrow37, eyeArrow38, eyeArrow39, eyeArrow40,
        eyeArrow41, eyeArrow42, eyeArrow43, eyeArrow44, eyeArrow45, eyeArrow46, eyeArrow47, eyeArrow48};
    const uint8_t boopLength = arrayLength(boopAnimation);
    TimeBasedAnimState boopAnim;
    TimeBasedAnimState boopLoopAnim;
    uint8_t boopLoopFrames = 10;  // Number of frames to loop at the end

    const uint8_t* oFaceAnimation[48] = {
        eyeCircle1, eyeCircle2, eyeCircle3, eyeCircle4, eyeCircle5, eyeCircle6, eyeCircle7, eyeCircle8, eyeCircle9, eyeCircle10,
        eyeCircle11, eyeCircle12, eyeCircle13, eyeCircle14, eyeCircle15, eyeCircle16, eyeCircle17, eyeCircle18, eyeCircle19, eyeCircle20,
        eyeCircle21, eyeCircle22, eyeCircle23, eyeCircle24, eyeCircle25, eyeCircle26, eyeCircle27, eyeCircle28, eyeCircle29, eyeCircle30,
        eyeCircle31, eyeCircle32, eyeCircle33, eyeCircle34, eyeCircle35, eyeCircle36, eyeCircle37, eyeCircle38, eyeCircle39, eyeCircle40,
        eyeCircle41, eyeCircle42, eyeCircle43, eyeCircle44, eyeCircle45, eyeCircle46, eyeCircle47, eyeCircle48};
    const uint8_t oFaceLength = arrayLength(oFaceAnimation);
    TimeBasedAnimState oFaceAnim;
    TimeBasedAnimState oFaceLoopAnim;
    uint8_t oFaceLoopFrames = 10;  // Number of frames to loop at the end

    const uint8_t* smileAnimation[48] = {
        eyeSmile1, eyeSmile2, eyeSmile3, eyeSmile4, eyeSmile5, eyeSmile6, eyeSmile7, eyeSmile8, eyeSmile9, eyeSmile10,
        eyeSmile11, eyeSmile12, eyeSmile13, eyeSmile14, eyeSmile15, eyeSmile16, eyeSmile17, eyeSmile18, eyeSmile19, eyeSmile20,
        eyeSmile21, eyeSmile22, eyeSmile23, eyeSmile24, eyeSmile25, eyeSmile26, eyeSmile27, eyeSmile28, eyeSmile29, eyeSmile30,
        eyeSmile31, eyeSmile32, eyeSmile33, eyeSmile34, eyeSmile35, eyeSmile36, eyeSmile37, eyeSmile38, eyeSmile39, eyeSmile40,
        eyeSmile41, eyeSmile42, eyeSmile43, eyeSmile44, eyeSmile45, eyeSmile46, eyeSmile47, eyeSmile48};
    const uint8_t smileLength = arrayLength(smileAnimation);

    TimeBasedAnimState smileAnim;
    TimeBasedAnimState smileLoopAnim;
    uint8_t smileLoopFrames = 10;  // Number of frames to loop at the end

    const uint8_t* eyeAngryAnimation[48] = {
        eyeAngry1, eyeAngry2, eyeAngry3, eyeAngry4, eyeAngry5, eyeAngry6, eyeAngry7, eyeAngry8, eyeAngry9, eyeAngry10,
        eyeAngry11, eyeAngry12, eyeAngry13, eyeAngry14, eyeAngry15, eyeAngry16, eyeAngry17, eyeAngry18, eyeAngry19, eyeAngry20,
        eyeAngry21, eyeAngry22, eyeAngry23, eyeAngry24, eyeAngry25, eyeAngry26, eyeAngry27, eyeAngry28, eyeAngry29, eyeAngry30,
        eyeAngry31, eyeAngry32, eyeAngry33, eyeAngry34, eyeAngry35, eyeAngry36, eyeAngry37, eyeAngry38, eyeAngry39, eyeAngry40,
        eyeAngry41, eyeAngry42, eyeAngry43, eyeAngry44, eyeAngry45, eyeAngry46, eyeAngry47, eyeAngry48};
    const uint8_t angryLength = arrayLength(eyeAngryAnimation);
    TimeBasedAnimState angryAnim;
    TimeBasedAnimState angryLoopAnim;
    uint8_t angryLoopFrames = 10;  // Number of frames to loop at the end

    const uint8_t* eyeSleepAnimation[48] = {
        eyeSleep1, eyeSleep2, eyeSleep3, eyeSleep4, eyeSleep5, eyeSleep6, eyeSleep7, eyeSleep8, eyeSleep9, eyeSleep10,
        eyeSleep11, eyeSleep12, eyeSleep13, eyeSleep14, eyeSleep15, eyeSleep16, eyeSleep17, eyeSleep18, eyeSleep19, eyeSleep20,
        eyeSleep21, eyeSleep22, eyeSleep23, eyeSleep24, eyeSleep25, eyeSleep26, eyeSleep27, eyeSleep28, eyeSleep29, eyeSleep30,
        eyeSleep31, eyeSleep32, eyeSleep33, eyeSleep34, eyeSleep35, eyeSleep36, eyeSleep37, eyeSleep38, eyeSleep39, eyeSleep40,
        eyeSleep41, eyeSleep42, eyeSleep43, eyeSleep44, eyeSleep45, eyeSleep46, eyeSleep47, eyeSleep48};
    const uint8_t sleepLength = arrayLength(eyeSleepAnimation);
    uint8_t sleepIndex = 0, sleepRand = 0;
    unsigned long startSleepTime;
    unsigned long lastSleepFrameChange = 0;  // Track last actual frame change (decoupled from check interval)
    int calculateSleepIndex(int currentIndex);

    void movingEye();
    void idleFace();
    void updateIdleMicroMovements();
    void checkAndTriggerBlink();
    unsigned long selectBlinkInterval();
    const TimeBasedAnimConfig* selectBlinkSpeed(bool isDoubleBlink);
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
