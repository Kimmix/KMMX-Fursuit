#pragma once
#include <Adafruit_Sensor.h>
#include "LEDMatrixDisplay.h"
#include "Bitmaps/bitmaps.h"
#include "Utils.h"
#include "Viseme.h"

enum class MouthStateEnum { IDLE,
                            BOOP,
                            ANGRYBOOP,
                            TALKING };

class MouthState {
   public:
    MouthState(LEDMatrixDisplay* display);
    Viseme viseme;
    void startMic();
    void update();
    void setState(MouthStateEnum newState);
    void savePrevState(MouthStateEnum newState);
    MouthStateEnum getState() const;
    void getListEvent(const sensors_event_t& eventData);

   private:
    LEDMatrixDisplay* display;
    sensors_event_t event;
    MouthStateEnum prevState, currentState = MouthStateEnum::IDLE;
    unsigned long mouthInterval, resetBoop, nextAngry;
    bool isTransitioning = false;
    const uint8_t *visemeFrame = mouthDefault, *mouthFrame = mouthDefault;

    const uint8_t* defaultAnimation[20] = {mouthDefault1, mouthDefault2, mouthDefault3, mouthDefault4, mouthDefault5, mouthDefault6, mouthDefault7, mouthDefault8, mouthDefault9, mouthDefault10, mouthDefault11, mouthDefault12, mouthDefault13, mouthDefault14, mouthDefault15, mouthDefault16, mouthDefault17, mouthDefault18, mouthDefault19, mouthDefault20};

    const uint8_t* mouthUp[20] = {mouthUp1, mouthUp2, mouthUp3, mouthUp4, mouthUp5, mouthUp6, mouthUp7, mouthUp8, mouthUp9, mouthUp10, mouthUp11, mouthUp12, mouthUp13, mouthUp14, mouthUp15, mouthUp16, mouthUp17, mouthUp18, mouthUp19, mouthUp20};

    const uint8_t* mouthDown[20] = {mouthDown1, mouthDown2, mouthDown3, mouthDown4, mouthDown5, mouthDown6, mouthDown7, mouthDown8, mouthDown9, mouthDown10, mouthDown11, mouthDown12, mouthDown13, mouthDown14, mouthDown15, mouthDown16, mouthDown17, mouthDown18, mouthDown19, mouthDown20};

    const uint8_t* mouthDown[20] = {mouthDown1, mouthDown2, mouthDown3, mouthDown4, mouthDown5, mouthDown6, mouthDown7, mouthDown8, mouthDown9, mouthDown10, mouthDown11, mouthDown12, mouthDown13, mouthDown14, mouthDown15, mouthDown16, mouthDown17, mouthDown18, mouthDown19, mouthDown20};

    const uint8_t* mouthAngryAnimation[20] = {mouthAH1, mouthAH2, mouthAH3, mouthAH4, mouthAH5, mouthAH6, mouthAH7, mouthAH8, mouthAH9, mouthAH10, mouthAH11, mouthAH12, mouthAH13, mouthAH14, mouthAH15, mouthAH16, mouthAH17, mouthAH18, mouthAH19, mouthAH20};
    short angryLength = arrayLength(mouthAngryAnimation);

    void drawDefault();
    void movingMouth();
    void updateAnimation();
    void updateIndex();
    void angryBoop();

    short defaultAnimationIndex = 0;
    bool increasingIndex = true;
    short angryIndex = 0;

    TaskHandle_t visemeTaskHandle = NULL;
    void startVisemeTask();
    static void visemeRenderingTask(void* parameter);
};
