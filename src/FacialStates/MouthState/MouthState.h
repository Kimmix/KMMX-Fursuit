#pragma once
#include <Adafruit_Sensor.h>
#include "Devices/LEDMatrixDisplay/Hub75DMA.h"
#include "Bitmaps/Bitmaps.h"
#include "Utils/Utils.h"
#include "Renderer/Viseme.h"
#include "Renderer/AnimationHelper.h"
#include "Types/SensorData.h"

enum class MouthStateEnum { IDLE,
                            BOOP,
                            ANGRYBOOP,
                            TALKING,
                            WAH };

class MouthState {
   public:
    MouthState(Hub75DMA* display);
    Viseme viseme;
    void startMic();
    void update();
    void setState(MouthStateEnum newState);
    void savePrevState(MouthStateEnum newState);
    MouthStateEnum getState() const;
    void setSensorData(const SensorData& data);
    void resetMovingMouth();
    void setSlowAnimation(bool slow);

   private:
    Hub75DMA* display;
    SensorData sensorData;
    MouthStateEnum prevState, currentState = MouthStateEnum::IDLE;
    unsigned long mouthInterval, resetBoop, nextAngry;
    bool isTransitioning = false;
    const uint8_t *visemeFrame = mouthDefault, *mouthFrame = mouthDefault;

    const uint8_t* defaultAnimation[60] = {mouthDefault1, mouthDefault2, mouthDefault3, mouthDefault4, mouthDefault5, mouthDefault6, mouthDefault7, mouthDefault8, mouthDefault9, mouthDefault10, mouthDefault11, mouthDefault12, mouthDefault13, mouthDefault14, mouthDefault15, mouthDefault16, mouthDefault17, mouthDefault18, mouthDefault19, mouthDefault20, mouthDefault21, mouthDefault22, mouthDefault23, mouthDefault24, mouthDefault25, mouthDefault26, mouthDefault27, mouthDefault28, mouthDefault29, mouthDefault30, mouthDefault31, mouthDefault32, mouthDefault33, mouthDefault34, mouthDefault35, mouthDefault36, mouthDefault37, mouthDefault38, mouthDefault39, mouthDefault40, mouthDefault41, mouthDefault42, mouthDefault43, mouthDefault44, mouthDefault45, mouthDefault46, mouthDefault47, mouthDefault48, mouthDefault49, mouthDefault50, mouthDefault51, mouthDefault52, mouthDefault53, mouthDefault54, mouthDefault55, mouthDefault56, mouthDefault57, mouthDefault58, mouthDefault59, mouthDefault60};
    short defaultAnimationLength = arrayLength(defaultAnimation);

    const uint8_t* mouthUp[60] = {mouthUp1, mouthUp2, mouthUp3, mouthUp4, mouthUp5, mouthUp6, mouthUp7, mouthUp8, mouthUp9, mouthUp10, mouthUp11, mouthUp12, mouthUp13, mouthUp14, mouthUp15, mouthUp16, mouthUp17, mouthUp18, mouthUp19, mouthUp20, mouthUp21, mouthUp22, mouthUp23, mouthUp24, mouthUp25, mouthUp26, mouthUp27, mouthUp28, mouthUp29, mouthUp30, mouthUp31, mouthUp32, mouthUp33, mouthUp34, mouthUp35, mouthUp36, mouthUp37, mouthUp38, mouthUp39, mouthUp40, mouthUp41, mouthUp42, mouthUp43, mouthUp44, mouthUp45, mouthUp46, mouthUp47, mouthUp48, mouthUp49, mouthUp50, mouthUp51, mouthUp52, mouthUp53, mouthUp54, mouthUp55, mouthUp56, mouthUp57, mouthUp58, mouthUp59, mouthUp60};

    const uint8_t* mouthDown[60] = {mouthDown1, mouthDown2, mouthDown3, mouthDown4, mouthDown5, mouthDown6, mouthDown7, mouthDown8, mouthDown9, mouthDown10, mouthDown11, mouthDown12, mouthDown13, mouthDown14, mouthDown15, mouthDown16, mouthDown17, mouthDown18, mouthDown19, mouthDown20, mouthDown21, mouthDown22, mouthDown23, mouthDown24, mouthDown25, mouthDown26, mouthDown27, mouthDown28, mouthDown29, mouthDown30, mouthDown31, mouthDown32, mouthDown33, mouthDown34, mouthDown35, mouthDown36, mouthDown37, mouthDown38, mouthDown39, mouthDown40, mouthDown41, mouthDown42, mouthDown43, mouthDown44, mouthDown45, mouthDown46, mouthDown47, mouthDown48, mouthDown49, mouthDown50, mouthDown51, mouthDown52, mouthDown53, mouthDown54, mouthDown55, mouthDown56, mouthDown57, mouthDown58, mouthDown59, mouthDown60};

    const uint8_t* mouthAngryAnimation[20] = {mouthAH1, mouthAH2, mouthAH3, mouthAH4, mouthAH5, mouthAH6, mouthAH7, mouthAH8, mouthAH9, mouthAH10, mouthAH11, mouthAH12, mouthAH13, mouthAH14, mouthAH15, mouthAH16, mouthAH17, mouthAH18, mouthAH19, mouthAH20};
    short angryLength = arrayLength(mouthAngryAnimation);

    const uint8_t* mouthWahAnimation[60] = {mouthWah1, mouthWah2, mouthWah3, mouthWah4, mouthWah5, mouthWah6, mouthWah7, mouthWah8, mouthWah9, mouthWah10, mouthWah11, mouthWah12, mouthWah13, mouthWah14, mouthWah15, mouthWah16, mouthWah17, mouthWah18, mouthWah19, mouthWah20, mouthWah21, mouthWah22, mouthWah23, mouthWah24, mouthWah25, mouthWah26, mouthWah27, mouthWah28, mouthWah29, mouthWah30, mouthWah31, mouthWah32, mouthWah33, mouthWah34, mouthWah35, mouthWah36, mouthWah37, mouthWah38, mouthWah39, mouthWah40, mouthWah41, mouthWah42, mouthWah43, mouthWah44, mouthWah45, mouthWah46, mouthWah47, mouthWah48, mouthWah49, mouthWah50, mouthWah51, mouthWah52, mouthWah53, mouthWah54, mouthWah55, mouthWah56, mouthWah57, mouthWah58, mouthWah59, mouthWah60};
    short wahLength = arrayLength(mouthWahAnimation);

    // Animation states
    AnimationState wahAnim;
    AnimationState idleAnim;

    void drawDefault();
    void movingMouth();
    void angryBoop();

    short angryIndex = 0;

    TaskHandle_t visemeTaskHandle = NULL;
    void startVisemeTask();
    static void visemeRenderingTask(void* parameter);
};
