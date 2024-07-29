#include "KMMXController.h"

void KMMXController::renderFace() {
    debugPixel(pixelPos);
    display.drawNose(noseNew);
    mouthState.update();
    eyeState.update();
    fxState.update();
    display.render();
    display.clearScreen();
}
