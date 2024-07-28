#include "Controller.h"

void Controller::renderFace() {
    debugPixel(pixelPos);
    display.drawNose(noseNew);
    mouthState.update();
    eyeState.update();
    fxState.update();
    display.render();
    display.clearScreen();
}
