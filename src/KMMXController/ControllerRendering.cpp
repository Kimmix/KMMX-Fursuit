#include "KMMXController.h"

void KMMXController::update() {
    statusLED.update();
    // cheekPanel.update();
    if (initBoop) {
        booping();
    }
    if (vsync) {
        if (millis() >= nextFrame) {
            nextFrame = millis() + frametime;
            renderFace();
        }
    } else {
        renderFace();
    }
}

void KMMXController::renderFace() {
    debugPixel(pixelPos);
    display.drawNose(noseNew);
    mouthState.update();
    eyeState.update();
    fxState.update();
    display.render();
    display.clearScreen();
}
