enum class EyeStateEnum { IDLE,
                          BLINK,
                          BOOP,
                          GOOGLY,
                          OEYE };
enum class MouthStateEnum { IDLE,
                            BOOP,
                            TALKING };

#include "Devices/LEDMatrixDisplay.h"
#include "Devices/SideLED.h"
#include "FacialStates/FacialState.h"
#include "FacialStates/MouthState.h"
#include "FacialStates/EyeState.h"
#include "RenderFunction/flyingHeart.h"
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
    FlyingHeart flyingHeart = FlyingHeart(&display);
    bool isBoop = false, isBoopPrev = false;

    void renderFace() {
        // display.drawColorTest();
        display.drawNose(noseNew);
        mouthState.update();
        eyeState.update();
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
            flyingHeart.reset();
        }
        isBoopPrev = isBoop;
        if (isBoop) {
            faceBoop();
            flyingHeart.renderHeart();
        }
    }

    // void getDynamicBoop() {
    //     float detectedSpeed = detectSpeed(analogRead(IR_PIN));
    //     if (detectedSpeed > 0.0) {
    //         Serial.print("Detected Speed: ");
    //         Serial.println(detectedSpeed);
    //     }
    // }

    enum SensorState {
        WAITING,
        DETECTED,
        TIMEOUT,
    };
    const int SENSOR_IN_RANGE_THRESHOLD = 500;    // Adjust this value based on your sensor characteristics
    const int SENSOR_OUT_RANGE_THRESHOLD = 4000;  // Adjust this value based on your sensor characteristics
    const unsigned long TIMEOUT_MS = 2000;        // Timeout in milliseconds (2 seconds)
    SensorState currentState = WAITING;
    unsigned long startTime = 0;
    void getDynamicBoop() {
        int sensorValue = analogRead(IR_PIN);
        switch (currentState) {
            case WAITING:
                if (sensorValue < SENSOR_OUT_RANGE_THRESHOLD && sensorValue > SENSOR_IN_RANGE_THRESHOLD) {
                    currentState = DETECTED;
                    startTime = millis();
                    flyingHeart.reset();
                }
                break;

            case DETECTED:
                mouthState.setState(MouthStateEnum::BOOP);
                if (sensorValue <= SENSOR_IN_RANGE_THRESHOLD) {
                    unsigned long elapsedTime = millis() - startTime;
                    float speed = map(elapsedTime, 100, TIMEOUT_MS * 0.75, 0, 100);
                    // speed = constrain(speed, 0.0, 1.0);
                    speed /= 100;
                    if (speed > 0.0) {
                        Serial.print("Detected Speed: ");
                        Serial.println(speed);
                        flyingHeart.setSpeed(speed);
                        flyingHeart.renderHeart();
                        eyeState.setState(EyeStateEnum::BOOP);
                        mouthState.setState(MouthStateEnum::BOOP);
                    }
                    currentState = WAITING;
                } else if (sensorValue >= SENSOR_OUT_RANGE_THRESHOLD) {
                    Serial.println("------ Out of Range ------");
                    currentState = WAITING;
                } else if (millis() - startTime >= TIMEOUT_MS) {
                    Serial.println("------ TIMED OUT ------");
                    currentState = TIMEOUT;
                }
                break;

            case TIMEOUT:
                currentState = WAITING;
                break;
        }
    }
};