#include "Devices/LEDMatrixDisplay.h"
#include "Devices/SideLED.h"
#include "Controller/eyeController.h"
#include "Controller/mouthController.h"
#include "Bitmaps/Icons.h"

class Controller {
   private:
    LEDMatrixDisplay display = LEDMatrixDisplay();
    SideLED sideLED;
    EyeState eyeState = EyeState(&display);
    MouthState mouthState = MouthState(&display);

   public:
    void render() {
        display.drawColorTest();
        display.drawNose(noseDefault);
        eyeState.update();
        sideLED.animate();
    }
    void render2() {
        mouthState.update();
    }

    void faceBoop() {
        eyeState.setBoop();
        mouthState.setBoop();
    }
};
