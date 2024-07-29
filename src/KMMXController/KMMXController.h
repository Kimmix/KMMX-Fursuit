#pragma once

#include "Arduino.h"
#include <Adafruit_LIS3DH.h>
// #include "Devices/APDS9930Sensor.h"
// #include "Devices/LIS3DH.h"
// #include "FacialStates/MouthState.h"
// #include "FacialStates/EyeState.h"
// #include "FacialStates/FXState.h"
// #include "RenderFunction/boop.h"
// #include "Bitmaps/Icons.h"

#include "config.h"
#include "Utils/Utils.h"

#include "FacialStates/EyeState/EyeState.h"
#include "FacialStates/MouthState/MouthState.h"
#include "FacialStates/FXState/FXState.h"

#include "Devices/LEDMatrixDisplay/Hub75DMA.h"
#include "Devices/HornLED/HornLED.h"
#include "Devices/SideLED/ARGBStrip.h"

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
    // LIS3DH accSensor;
    // APDS9930Sensor proxSensor;
    Hub75DMA display;
    ARGBStrip sideLED;
    HornLED hornLED;
    sensors_event_t *sensorEvent;
    TaskHandle_t sensorEventTaskHandle;
    // setup renderer state
    EyeState eyeState = EyeState(&display);
    MouthState mouthState = MouthState(&display);
    FXState fxState = FXState(&display);
    // Boop boop;
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
    bool isSleeping = false;
    unsigned long stillTime = 0;  // Time when the accelerometer became still
    unsigned long nextFrame;
    unsigned long nextBoop = 0;
    bool inRange = false, isBoop = false, isContinuous = false, isAngry = false;
    float boopSpeed = 0.0;
    unsigned long nextRead;
};