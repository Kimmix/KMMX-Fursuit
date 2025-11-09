#include "KMMXController.h"

void KMMXController::update() {
    statusLED.update();
    cheekPanel.update();
    hornLED.update();
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
    display.render();
    delay(1000 / display.getRefreshRate());
    display.clearScreen();\
    debugPixel(pixelPos);
    display.drawNose(noseNew);
    mouthState.update();
    eyeState.update();
    fxState.update();
}
