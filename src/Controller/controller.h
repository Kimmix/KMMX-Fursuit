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
   private:
    LEDMatrixDisplay display;
    SideLED sideLED;
    EyeState eyeState = EyeState(&display);
    MouthState mouthState = MouthState(&display);
    FlyingHeart flyingHeart = FlyingHeart(&display);
    bool isBoop = false, isBoopPrev = false;

   public:
    void update() {
        getBoop();
        renderFace();
        // sideLED.animate();
    }

    void getBoop() {
        isBoop = !digitalRead(IR_PIN);
        if (isBoopPrev == HIGH && isBoop == LOW) {
            resetBoop();
        }
        isBoopPrev = isBoop;
        if (isBoop) {
            faceBoop();
        }
    }

    void renderFace() {
        // display.drawColorTest();
        display.drawNose(noseNew);
        mouthState.update();
        eyeState.update();
        if (isBoop) {
            flyingHeart.renderHeart();
        }
        // Double Buffering
        display.render();
        display.clearScreen();
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

    void resetFace() {
        eyeState.setState(EyeStateEnum::IDLE);
        mouthState.setState(MouthStateEnum::IDLE);
    }

    void faceBoop() {
        if (isBoopPrev == LOW && isBoop == HIGH) {
            eyeState.setPrevState(eyeState.getState());
            mouthState.setPrevState(mouthState.getState());
        }
        eyeState.setState(EyeStateEnum::BOOP);
        mouthState.setState(MouthStateEnum::BOOP);
    }

    void resetBoop() {
        flyingHeart.reset();
    }
};