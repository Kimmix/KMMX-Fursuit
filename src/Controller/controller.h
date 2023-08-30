#include <Adafruit_LIS3DH.h>
#include "Devices/LEDMatrixDisplay.h"
#include "Devices/SideLED.h"
#include "FacialStates/MouthState.h"
#include "FacialStates/EyeState.h"
#include "FacialStates/FXState.h"
#include "Bitmaps/Icons.h"

#define IR_PIN GPIO_NUM_35
#define RANDOM_PIN GPIO_NUM_36
class Controller {
   public:
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

    void update() {
        // dynamicBoop();
        renderFace();
        sideLED.animate();
    }

    void setEye(int i) {
        switch (i) {
            case 1:
                eyeState.setState(EyeStateEnum::GOOGLY);
                break;
            case 2:
                eyeState.setState(EyeStateEnum::OEYE);
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
                mouthState.viseme.setNoiseThreshold(200);
                break;
            case 3:
                mouthState.viseme.setNoiseThreshold(400);
                break;
            case 4:
                mouthState.viseme.setNoiseThreshold(800);
                break;
            case 5:
                mouthState.viseme.setNoiseThreshold(1600);
                break;
            case 6:
                mouthState.viseme.setNoiseThreshold(3200);
                break;
            case 7:
                mouthState.viseme.setNoiseThreshold(4800);
                break;
            case 8:
                mouthState.viseme.setNoiseThreshold(6400);
                break;
            case 9:
                mouthState.viseme.setNoiseThreshold(8000);
                break;
            case 10:
                mouthState.viseme.setNoiseThreshold(12000);
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

   private:
    Adafruit_LIS3DH lis = Adafruit_LIS3DH();
    LEDMatrixDisplay display;
    SideLED sideLED;
    EyeState eyeState = EyeState(&display);
    MouthState mouthState = MouthState(&display);
    FXState fxState = FXState(&display);
    sensors_event_t sensorEvent;
    bool isBoop = false, isBoopPrev = false;

    void renderFace() {
        display.drawColorTest();
        display.drawNose(noseNew);
        mouthState.update();
        eyeState.update();
        fxState.update();
        // Double Buffering
        display.render();
        display.clearScreen();
    }

    enum BoopState {
        IDLE,
        BOOP_IN_PROGRESS,
    };
    BoopState currentBoopState = IDLE;
    const int IR_IN_RANGE_THRESHOLD = 300, IR_OUT_RANGE_THRESHOLD = 3600;
    const unsigned long boopDuration = 1000;
    unsigned long boopStartTime = 0;
    TaskHandle_t lisEventTaskHandle;

    void dynamicBoop() {
        int sensorValue = analogRead(IR_PIN);
        switch (currentBoopState) {
            case IDLE:
                if (sensorValue < IR_OUT_RANGE_THRESHOLD && sensorValue > IR_IN_RANGE_THRESHOLD) {
                    boopStartTime = millis();
                    currentBoopState = BOOP_IN_PROGRESS;
                }
                break;
            case BOOP_IN_PROGRESS:
                mouthState.setState(MouthStateEnum::BOOP);
                if (sensorValue <= IR_IN_RANGE_THRESHOLD) {
                    float speed = calculateBoopSpeed();
                    if (speed > 0.0) {
                        setBoopEffects(speed);
                    }
                    currentBoopState = IDLE;
                } else if (sensorValue >= IR_OUT_RANGE_THRESHOLD) {
                    currentBoopState = IDLE;
                }
                break;
        }
    }
    float calculateBoopSpeed() {
        unsigned long elapsedTime = millis() - boopStartTime;
        float speed = map(elapsedTime, 100, boopDuration, 0, 100);
        return speed / 100;
    }
    void setBoopEffects(float speed) {
        fxState.setFlyingSpeed(speed);
        fxState.setState(FXStateEnum::Heart);
        eyeState.setState(EyeStateEnum::BOOP);
        mouthState.setState(MouthStateEnum::BOOP);
    }
};