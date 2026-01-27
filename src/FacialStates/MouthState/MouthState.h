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

    const uint8_t* defaultAnimation[60] = {mouthDefault1_rle, mouthDefault2_rle, mouthDefault3_rle, mouthDefault4_rle, mouthDefault5_rle, mouthDefault6_rle, mouthDefault7_rle, mouthDefault8_rle, mouthDefault9_rle, mouthDefault10_rle, mouthDefault11_rle, mouthDefault12_rle, mouthDefault13_rle, mouthDefault14_rle, mouthDefault15_rle, mouthDefault16_rle, mouthDefault17_rle, mouthDefault18_rle, mouthDefault19_rle, mouthDefault20_rle, mouthDefault21_rle, mouthDefault22_rle, mouthDefault23_rle, mouthDefault24_rle, mouthDefault25_rle, mouthDefault26_rle, mouthDefault27_rle, mouthDefault28_rle, mouthDefault29_rle, mouthDefault30_rle, mouthDefault31_rle, mouthDefault32_rle, mouthDefault33_rle, mouthDefault34_rle, mouthDefault35_rle, mouthDefault36_rle, mouthDefault37_rle, mouthDefault38_rle, mouthDefault39_rle, mouthDefault40_rle, mouthDefault41_rle, mouthDefault42_rle, mouthDefault43_rle, mouthDefault44_rle, mouthDefault45_rle, mouthDefault46_rle, mouthDefault47_rle, mouthDefault48_rle, mouthDefault49_rle, mouthDefault50_rle, mouthDefault51_rle, mouthDefault52_rle, mouthDefault53_rle, mouthDefault54_rle, mouthDefault55_rle, mouthDefault56_rle, mouthDefault57_rle, mouthDefault58_rle, mouthDefault59_rle, mouthDefault60_rle};
    short defaultAnimationLength = arrayLength(defaultAnimation);

    const uint8_t* mouthUp[20] = {mouthUp1, mouthUp2, mouthUp3, mouthUp4, mouthUp5, mouthUp6, mouthUp7, mouthUp8, mouthUp9, mouthUp10, mouthUp11, mouthUp12, mouthUp13, mouthUp14, mouthUp15, mouthUp16, mouthUp17, mouthUp18, mouthUp19, mouthUp20};

    const uint8_t* mouthDown[20] = {mouthDown1, mouthDown2, mouthDown3, mouthDown4, mouthDown5, mouthDown6, mouthDown7, mouthDown8, mouthDown9, mouthDown10, mouthDown11, mouthDown12, mouthDown13, mouthDown14, mouthDown15, mouthDown16, mouthDown17, mouthDown18, mouthDown19, mouthDown20};

    const uint8_t* mouthAngryAnimation[20] = {mouthAH1, mouthAH2, mouthAH3, mouthAH4, mouthAH5, mouthAH6, mouthAH7, mouthAH8, mouthAH9, mouthAH10, mouthAH11, mouthAH12, mouthAH13, mouthAH14, mouthAH15, mouthAH16, mouthAH17, mouthAH18, mouthAH19, mouthAH20};
    short angryLength = arrayLength(mouthAngryAnimation);

    const uint8_t* mouthWahAnimation[60] = {mouthWah1_rle, mouthWah2_rle, mouthWah3_rle, mouthWah4_rle, mouthWah5_rle, mouthWah6_rle, mouthWah7_rle, mouthWah8_rle, mouthWah9_rle, mouthWah10_rle, mouthWah11_rle, mouthWah12_rle, mouthWah13_rle, mouthWah14_rle, mouthWah15_rle, mouthWah16_rle, mouthWah17_rle, mouthWah18_rle, mouthWah19_rle, mouthWah20_rle, mouthWah21_rle, mouthWah22_rle, mouthWah23_rle, mouthWah24_rle, mouthWah25_rle, mouthWah26_rle, mouthWah27_rle, mouthWah28_rle, mouthWah29_rle, mouthWah30_rle, mouthWah31_rle, mouthWah32_rle, mouthWah33_rle, mouthWah34_rle, mouthWah35_rle, mouthWah36_rle, mouthWah37_rle, mouthWah38_rle, mouthWah39_rle, mouthWah40_rle, mouthWah41_rle, mouthWah42_rle, mouthWah43_rle, mouthWah44_rle, mouthWah45_rle, mouthWah46_rle, mouthWah47_rle, mouthWah48_rle, mouthWah49_rle, mouthWah50_rle, mouthWah51_rle, mouthWah52_rle, mouthWah53_rle, mouthWah54_rle, mouthWah55_rle, mouthWah56_rle, mouthWah57_rle, mouthWah58_rle, mouthWah59_rle, mouthWah60_rle};
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
