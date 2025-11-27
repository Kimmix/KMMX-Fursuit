#pragma once
#include <Arduino.h>

#include "config.h"
#include "Utils/Utils.h"

#include "FacialStates/EyeState/EyeState.h"
#include "FacialStates/MouthState/MouthState.h"
#include "FacialStates/FXState/FXState.h"

#include "Renderer/Boop.h"

#include "Devices/LEDMatrixDisplay/Hub75DMA.h"
#include "Devices/HornLED/HornLED.h"
#include "Devices/Accelerometer/LIS3DH.h"
#include "Devices/Proximity/APDS9930Sensor.h"
#include "Devices/Ws2812/RGBStatus.h"
#include "Devices/Ws2812/CheekPanel.h"

class KMMXController {
   public:
    void setupSensors();
    void update();
    void setEye(int i);
    void setViseme(int b);
    int getViseme();
    int getDisplayBrightness();
    void setDisplayBrightness(int i);

   private:
    // Devices
    Hub75DMA display;
    RGBStatus statusLED = RGBStatus(RGB_STATUS_PIN);
    CheekPanel cheekPanel = CheekPanel(argbCount, ARGB_PIN);
    HornLED hornLED;
    LIS3DH accelerometer;
    APDS9930Sensor proximitySensor;
    sensors_event_t *sensorEvent;
    TaskHandle_t sensorTaskHandle;
    // Renderer states
    EyeState eyeState = EyeState(&display);
    MouthState mouthState = MouthState(&display);
    FXState fxState = FXState(&display);
    Boop boop;
    int16_t pixelPos = 0;
    uint16_t proximityValue;

    void renderFace();
    void booping();
    void updateSensorValues();
    void resetIdletime(KMMXController *controller);
    void resetIdletime();
    void sleep(KMMXController *controller);
    void checkIdleAndSleep(KMMXController *controller, unsigned long currentTime);
    static void readSensorTask(void *parameter);

    float lastAccelX, lastAccelY, lastAccelZ;
    float prevAccelX, prevAccelY, prevAccelZ;
    unsigned long stillTime = 0;  // Time when the accelerometer became still
    unsigned long nextFrame;
    unsigned long nextBoop = 0;
    bool isSleeping = false;
    bool boopInitialized = false, inBoopRange = false, isBooping = false, isContinuousBoop = false, isAngry = false;
    unsigned short prevHornBright = hornInitBrightness;
    float boopSpeed = 0.0f;
};