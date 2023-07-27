#include "Devices/LEDMatrixDisplay.h"
#include "Devices/SideLED.h"
#include "FacialStates/FacialState.h"
#include "FacialStates/MouthState.h"
#include "FacialStates/EyeState.h"
#include "Bitmaps/Icons.h"

class Controller {
   private:
    SideLED sideLED;
    LEDMatrixDisplay display;
    EyeState eyeState = EyeState(&display);
    MouthState mouthState = MouthState(&display);

   public:
    void render() {
        // sideLED.animate();
        display.drawColorTest();
        display.drawNose(noseDefault);
        mouthState.update();
        eyeState.update();
        display.render();
        display.clearScreen();
    }

    void faceBoop() {
        eyeState.setState(EyeStateEnum::BOOP);
        mouthState.setState(MouthStateEnum::BOOP);
    }
};