#include <Adafruit_LIS3DH.h>
#include "Devices/LEDMatrixDisplay.h"
#include "Devices/SideLED.h"
#include "Devices/HornLED.h"
#include "Devices/APDS9930Sensor.h"
#include "Devices/LIS3DH.h"
#include "FacialStates/MouthState.h"
#include "FacialStates/EyeState.h"
#include "FacialStates/FXState.h"
#include "RenderFunction/boop.h"
#include "Bitmaps/Icons.h"

#define VSYNC false
class Controller {
   public:
    void setupSensors() {
        // mouthState.startMic();
        accSensor.setUp();
        proxSensor.setup();
        xTaskCreatePinnedToCore(readSensor, "SensorEventTask", 4096, this, 1, &sensorEventTaskHandle, 0);
    }

    unsigned long nextFrame;
    const short frametime = 7;  // ~144hz
    void update() {
        sideLED.animate();
        booping();
        if (VSYNC) {
            if (millis() >= nextFrame) {
                nextFrame = millis() + frametime;
                renderFace();
            }
        } else {
            renderFace();
        }
    }

    void setEye(int i) {
        switch (i) {
            case 1:
                eyeState.setState(EyeStateEnum::GOOGLY);
                break;
            case 2:
                eyeState.setState(EyeStateEnum::HEART);
                break;
            case 3:
                eyeState.setState(EyeStateEnum::SMILE);
                break;
            default:
                eyeState.setState(EyeStateEnum::IDLE);
                break;
        }
    }

    void setViseme(int b) {
        static const int noiseThresholds[] = {400, 600, 1000, 1600, 3200, 6400, 1200, 18000, 25000};
        static const int numThresholds = sizeof(noiseThresholds) / sizeof(noiseThresholds[0]);
        if (b == 0) {
            mouthState.setState(MouthStateEnum::IDLE);
        } else if (b == 1 || b > numThresholds) {
            mouthState.setState(MouthStateEnum::TALKING);
        } else {
            mouthState.viseme.setNoiseThreshold(noiseThresholds[b - 2]);
        }
    }

    int getViseme() {
        return mouthState.getState() == MouthStateEnum::TALKING;
    }

    int getDisplayBrightness() {
        return display.getBrightnessValue();
    }

    void setDisplayBrightness(int i) {
        display.setBrightnessValue(i);
    }

    void recieveEspNow(int16_t data) {
        switch (data) {
            case 1:
                eyeState.setState(EyeStateEnum::IDLE);
                break;
            case 2:
                eyeState.setState(EyeStateEnum::SMILE);
                break;
            case 3:
                eyeState.setState(EyeStateEnum::OEYE);
                break;
            default:
                break;
        }
    }

    void updatePixelPosition(int16_t y) {
        Serial.println(y, BIN);
        pixelPos = y;
    }

    void debugPixel(int16_t p) {
        int bit_position = 0;
        while (p > 0) {
            if (p & 1) {
                display.drawPixel(0, bit_position, display.color565(0, 200, 255));
            }
            p >>= 1;
            bit_position++;
        }
    }

   private:
    // Setup devices
    LIS3DH accSensor;
    APDS9930Sensor proxSensor;
    LEDMatrixDisplay display;
    SideLED sideLED;
    HornLED hornLED;
    sensors_event_t *sensorEvent;
    TaskHandle_t sensorEventTaskHandle;
    // setup renderer state
    EyeState eyeState = EyeState(&display);
    MouthState mouthState = MouthState(&display);
    FXState fxState = FXState(&display);
    Boop boop;
    int16_t pixelPos = 0;
    uint16_t proxValue;

    void renderFace() {
        debugPixel(pixelPos);
        // display.drawColorTest();
        display.drawNose(noseNew);
        mouthState.update();
        eyeState.update();
        fxState.update();
        // Double Buffering
        display.render();
        display.clearScreen();
    }

    bool inRange = false, isBoop = false, isContinuous = false, isAngry = false;
    float boopSpeed = 0.0;
    unsigned long nextBoop = 0;
    void booping() {
        if (millis() >= nextBoop) {
            nextBoop = millis() + 100;
            boop.getBoop(proxValue, inRange, isBoop, boopSpeed, isContinuous, isAngry);
            if (isBoop) {
                // Serial.println(boopSpeed);
                fxState.setFlyingSpeed(boopSpeed);
                fxState.setState(FXStateEnum::Heart);
                eyeState.setState(EyeStateEnum::BOOP);
                mouthState.setState(MouthStateEnum::BOOP);
            } else if (inRange) {
                mouthState.setState(MouthStateEnum::BOOP);
                resetIdletime();
            } else if (isContinuous) {
                eyeState.setState(EyeStateEnum::BOOP);
            } else if (isAngry) {
                nextBoop = millis() + 1500;
                eyeState.setState(EyeStateEnum::ANGRY);
                mouthState.setState(MouthStateEnum::ANGRYBOOP);
                resetIdletime();
            }
        }
    }

    void sleep() {
        // Implement your sleep function here
        Serial.println("Going to sleep...");
    }

    float lastX, lastY, lastZ;
    float prevX, prevY, prevZ;
    void updateSensorValues() {
        // Get proxSensor value
        proxSensor.read(&proxValue);
        sensorEvent = accSensor.getSensorEvent();
        // Store the current acceleration values
        prevX = lastX;
        prevY = lastY;
        prevZ = lastZ;

        // Update the last acceleration values
        lastX = sensorEvent->acceleration.x;
        lastY = sensorEvent->acceleration.y;
        lastZ = sensorEvent->acceleration.z;
    }

    void resetIdletime() {
        stillTime = 0;
    }

    const float THRESHOLD = 0.6;
    unsigned long stillTime = 0;  // Time when the accelerometer became still
    void checkIdleAndSleep(unsigned long currentTime) {
        if (abs(lastX - prevX) < THRESHOLD &&
            abs(lastY - prevY) < THRESHOLD &&
            abs(lastZ - prevZ) < THRESHOLD) {
            if (stillTime == 0) {
                stillTime = currentTime;
            } else if (currentTime - stillTime >= 30000) {
                sleep();
                resetIdletime();
            }
        } else {
            Serial.println("Oh Hai");
            resetIdletime();
        }
    }

    unsigned long nextRead;
    static void readSensor(void *parameter) {
        Controller *controller = static_cast<Controller *>(parameter);
        while (1) {
            unsigned long currentTime = millis();
            if (currentTime >= controller->nextRead) {
                controller->nextRead = currentTime + 100;
                controller->updateSensorValues();
                controller->checkIdleAndSleep(currentTime);
            }
            controller->mouthState.getListEvent(*(controller->sensorEvent));
            controller->eyeState.getListEvent(*(controller->sensorEvent));
        }
    }
};