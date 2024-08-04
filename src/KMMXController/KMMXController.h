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
    void recieveEspNow(int16_t data);
    void updatePixelPosition(int16_t y);
    void debugPixel(int16_t p);

   private:
    // Setup devices
    Hub75DMA display;
    RGBStatus statusLED = RGBStatus(RGB_BUILTIN);
    CheekPanel cheekPanel = CheekPanel(argbCount, ARGB_PIN);
    HornLED hornLED;
    LIS3DH accSensor;
    APDS9930Sensor proxSensor;
    sensors_event_t *sensorEvent;
    TaskHandle_t sensorEventTaskHandle;
    // setup renderer state
    EyeState eyeState = EyeState(&display);
    MouthState mouthState = MouthState(&display);
    FXState fxState = FXState(&display);
    Boop boop;
    int16_t pixelPos = 0;
    uint16_t proxValue;

    void renderFace();
    void booping();
    void updateSensorValues();
    void resetIdletime(KMMXController *controller);
    void resetIdletime();
    void sleep(KMMXController *controller);
    void checkIdleAndSleep(KMMXController *controller, unsigned long currentTime);
    static void readSensor(void *parameter);

    float lastX, lastY, lastZ;
    float prevX, prevY, prevZ;
    unsigned long stillTime = 0;  // Time when the accelerometer became still
    unsigned long nextFrame;
    unsigned long nextBoop = 0;
    bool isSleeping = false;
    bool initBoop = false, inRange = false, isBoop = false, isContinuous = false, isAngry = false;
    float boopSpeed = 0.0;
    unsigned long nextRead;
};