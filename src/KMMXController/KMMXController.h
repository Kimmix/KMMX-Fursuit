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
#include "Types/SensorData.h"

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
    // Double-buffer for thread-safe sensor access
    SensorData sensorBuffer[2];
    volatile uint8_t activeBuffer = 0;
    TaskHandle_t sensorTaskHandle;
    TaskHandle_t renderTaskHandle;
    // Renderer states
    EyeState eyeState = EyeState(&display);
    MouthState mouthState = MouthState(&display);
    FXState fxState = FXState(&display);
    Boop boop;
    int16_t pixelPos = 0;

    void renderFace();
    void booping();
    void resetIdletime(KMMXController *controller);
    void resetIdletime();
    void sleep(KMMXController *controller);
    void checkIdleAndSleep(KMMXController *controller, unsigned long currentTime);
    static void readSensorTask(void *parameter);
    static void renderTask(void *parameter);
    const SensorData& getSensorData() const;

    // Previous sensor values for idle detection
    SensorData prevSensorData;
    unsigned long stillTime = 0;  // Time when the accelerometer became still
    unsigned long nextFrame;
    unsigned long nextBoop = 0;
    bool isSleeping = false;
    bool boopInitialized = false, inBoopRange = false, isBooping = false, isContinuousBoop = false, isAngry = false;
    unsigned short prevHornBright = hornInitBrightness;
    float boopSpeed = 0.0f;
};