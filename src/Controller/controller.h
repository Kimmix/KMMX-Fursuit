enum class EyeStateEnum { IDLE,
                          BLINK,
                          BOOP,
                          GOOGLY,
                          OEYE };
enum class MouthStateEnum { IDLE,
                            BOOP,
                            TALKING };
enum class FXStateEnum { IDLE,
                         Heart,
                         Blush };

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
    void update() {
        // getBoop();
        getDynamicBoop();
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

    void setMouth(int i) {
        switch (i) {
            case 1:
                mouthState.setState(MouthStateEnum::TALKING);
                break;
            default:
                mouthState.setState(MouthStateEnum::IDLE);
                break;
        }
    }

    void setDisplayBrightness(int i) {
        display.setBrightnessValue(i);
    }

   private:
    LEDMatrixDisplay display;
    SideLED sideLED;
    EyeState eyeState = EyeState(&display);
    MouthState mouthState = MouthState(&display);
    FXState fxState = FXState(&display);
    bool isBoop = false, isBoopPrev = false;

    void renderFace() {
        // display.drawColorTest();
        display.drawNose(noseNew);
        mouthState.update();
        eyeState.update();
        fxState.update();
        // Double Buffering
        display.render();
        display.clearScreen();
    }

    void faceBoop() {
        eyeState.setState(EyeStateEnum::BOOP);
        mouthState.setState(MouthStateEnum::BOOP);
    }

    void getBoop() {
        isBoop = !digitalRead(IR_PIN);
        if (isBoopPrev == LOW && isBoop == HIGH) {
            eyeState.setPrevState(eyeState.getState());
            mouthState.setPrevState(mouthState.getState());
        }
        if (isBoopPrev == HIGH && isBoop == LOW) {
            // flyingHeart.reset();
        }
        isBoopPrev = isBoop;
        if (isBoop) {
            faceBoop();
            // flyingHeart.renderHeart();
        }
    }

    enum SensorState {
        WAITING,
        DETECTED,
        TIMEOUT,
    };
    const int SENSOR_IN_RANGE_THRESHOLD = 300;    // Adjust this value based on your sensor characteristics
    const int SENSOR_OUT_RANGE_THRESHOLD = 3900;  // Adjust this value based on your sensor characteristics
    const unsigned long TIMEOUT_MS = 2000;        // Timeout in milliseconds (2 seconds)
    SensorState currentState = WAITING;
    unsigned long startTime = 0;

    void getDynamicBoop() {
        int sensorValue = analogRead(IR_PIN);
        if (sensorValue < SENSOR_OUT_RANGE_THRESHOLD && sensorValue > SENSOR_IN_RANGE_THRESHOLD) {
            startTime = millis();
            while (sensorValue < SENSOR_OUT_RANGE_THRESHOLD && sensorValue > SENSOR_IN_RANGE_THRESHOLD) {
                sensorValue = analogRead(IR_PIN);
                mouthState.setState(MouthStateEnum::BOOP);
                if (sensorValue <= SENSOR_IN_RANGE_THRESHOLD) {
                    unsigned long elapsedTime = millis() - startTime;
                    float speed = map(elapsedTime, 100, TIMEOUT_MS * 0.75, 0, 100);
                    speed /= 100;
                    if (speed > 0.0) {
                        fxState.setFlyingSpeed(speed);
                        fxState.setState(FXStateEnum::Heart);
                        eyeState.setState(EyeStateEnum::BOOP);
                        mouthState.setState(MouthStateEnum::BOOP);
                    }
                    return;
                }
            }
            return;
        }
    }
};