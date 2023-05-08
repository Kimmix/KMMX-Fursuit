#include "Bitmaps/Icons.h"

#include "Devices/LEDMatrixDisplay.h"
#include "Controller/eyeController.h"
#include "Controller/mouthController.h"

class Controller {
   private:
    LEDMatrixDisplay display = LEDMatrixDisplay();
    EyeState eyeState = EyeState(&display);
    MouthState mouthState = MouthState(&display);

   public:
    void render() {
        display.drawNose(noseDefault);
        eyeState.update();
        mouthState.update();
    }

    void faceBoop() {
        eyeState.setBoop();
        mouthState.setBoop();
    }
};
