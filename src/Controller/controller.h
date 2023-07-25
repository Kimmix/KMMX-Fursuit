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
        // sideLED.animate();
        display.drawColorTest();
        display.drawNose(noseDefault);
        mouthState.update();
        eyeState.update();
        display.render();
        display.clearScreen();
    }

    void faceBoop() {
        eyeState.setBoop();
        mouthState.setBoop();
    }
};