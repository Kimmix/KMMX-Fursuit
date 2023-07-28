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
#include "Bitmaps/Icons.h"

#define IR_PIN GPIO_NUM_35
#define RANDOM_PIN GPIO_NUM_36
class Controller {
   private:
    LEDMatrixDisplay display;
    SideLED sideLED;
    EyeState eyeState = EyeState(&display);
    MouthState mouthState = MouthState(&display);

   public:
    void update() {
        renderFace();
        // sideLED.animate();
    }

    void renderFace() {
        display.drawColorTest();
        display.drawNose(noseNew);
        mouthState.update();
        eyeState.update();
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

    void resetFace() {
        eyeState.setState(EyeStateEnum::IDLE);
        mouthState.setState(MouthStateEnum::IDLE);
    }

    void faceBoop() {
        eyeState.setPrevState(eyeState.getState());
        eyeState.setState(EyeStateEnum::BOOP);
        mouthState.setState(MouthStateEnum::BOOP);
    }
};