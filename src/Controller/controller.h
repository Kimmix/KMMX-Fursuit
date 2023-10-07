#include <Adafruit_LIS3DH.h>
#include "Devices/LEDMatrixDisplay.h"
#include "Devices/SideLED.h"
#include "FacialStates/MouthState.h"
#include "FacialStates/EyeState.h"
#include "FacialStates/FXState.h"
#include "RenderFunction/boop.h"
#include "Bitmaps/Icons.h"

#define RANDOM_PIN GPIO_NUM_36
class Controller {
   public:
    void setupSensors() {
        mouthState.startMic();
        setUpLis();
    }

    unsigned long nextFrame;
    const short frametime = 7;  // ~144hz
    void update() {
        booping();
        sideLED.animate();
        if (millis() >= nextFrame) {
            nextFrame = millis() + frametime;
            renderFace();
            // showFPS();
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
        switch (b) {
            case 0:
                mouthState.setState(MouthStateEnum::IDLE);
                break;
            case 1:
                mouthState.setState(MouthStateEnum::TALKING);
                break;
            case 2:
                mouthState.viseme.setNoiseThreshold(400);
                break;
            case 3:
                mouthState.viseme.setNoiseThreshold(600);
                break;
            case 4:
                mouthState.viseme.setNoiseThreshold(1000);
                break;
            case 5:
                mouthState.viseme.setNoiseThreshold(1600);
                break;
            case 6:
                mouthState.viseme.setNoiseThreshold(3200);
                break;
            case 7:
                mouthState.viseme.setNoiseThreshold(6400);
                break;
            case 8:
                mouthState.viseme.setNoiseThreshold(1200);
                break;
            case 9:
                mouthState.viseme.setNoiseThreshold(18000);
                break;
            case 10:
                mouthState.viseme.setNoiseThreshold(25000);
                break;
            default:
                mouthState.setState(MouthStateEnum::TALKING);
                break;
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
                display.drawPixel(124, bit_position, display.color565(0, 200, 255));
            }
            p >>= 1;
            bit_position++;
        }
    }

   private:
    Adafruit_LIS3DH lis = Adafruit_LIS3DH();
    LEDMatrixDisplay display;
    SideLED sideLED;
    Boop boop;
    EyeState eyeState = EyeState(&display);
    MouthState mouthState = MouthState(&display);
    FXState fxState = FXState(&display);
    sensors_event_t sensorEvent;
    TaskHandle_t lisEventTaskHandle;
    int16_t pixelPos = 0;

    void renderFace() {
        debugPixel(pixelPos);
        display.drawColorTest();
        display.drawNose(noseNew);
        mouthState.update();
        eyeState.update();
        fxState.update();
        // Double Buffering
        display.render();
        display.clearScreen();
    }

    bool inRange = false, isBoop = false;
    float boopSpeed = 0.0;
    unsigned long nextBoop = 0;
    void booping() {
        if (millis() >= nextBoop) {
            nextBoop = millis() + 100;
            boop.getBoop(inRange, isBoop, boopSpeed);
            if (isBoop) {
                Serial.println(boopSpeed);
                fxState.setFlyingSpeed(boopSpeed);
                fxState.setState(FXStateEnum::Heart);
                eyeState.setState(EyeStateEnum::BOOP);
                mouthState.setState(MouthStateEnum::BOOP);
            } else if (inRange) {
                mouthState.setState(MouthStateEnum::BOOP);
            }
        }
    }

    //? -------- LIS3DH --------
    void setUpLis() {
        if (!lis.begin(0x19)) {
            Serial.println("Could not initialize LIS3DH");
            while (1)
                ;
        }
        lis.setDataRate(LIS3DH_DATARATE_25_HZ);
        lis.setRange(LIS3DH_RANGE_2_G);
        xTaskCreatePinnedToCore(lisEventTask, "LisEventTask", 2048, this, 1, &lisEventTaskHandle, 0);
    }

    void getLisEvent() {
        lis.getEvent(&sensorEvent);
        // Serial.print("X:");
        // Serial.print(sensorEvent.acceleration.x);
        // Serial.print(",Y:");
        // Serial.print(sensorEvent.acceleration.y);
        // Serial.print(",Z:");
        // Serial.println(sensorEvent.acceleration.z);
        mouthState.getListEvent(sensorEvent);
        eyeState.getListEvent(sensorEvent);
    }

    static void lisEventTask(void *parameter) {
        Controller *controller = static_cast<Controller *>(parameter);
        while (1) {
            controller->getLisEvent();
        }
    }
};