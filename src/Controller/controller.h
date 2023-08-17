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
        dynamicBoop();
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

    void setViseme(boolean b) {
        switch (b) {
            case true:
                mouthState.setState(MouthStateEnum::TALKING);
                break;
            default:
                mouthState.setState(MouthStateEnum::IDLE);
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

    enum BoopState {
        IDLE,
        BOOP_IN_PROGRESS,
    };
    BoopState currentBoopState = IDLE;
    const int IR_IN_RANGE_THRESHOLD = 300, IR_OUT_RANGE_THRESHOLD = 3600;
    const unsigned long boopDuration = 1000;
    unsigned long boopStartTime = 0;

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