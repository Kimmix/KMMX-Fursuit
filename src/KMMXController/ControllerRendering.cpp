#include "KMMXController.h"

void KMMXController::update() {
    statusLED.update();
    cheekPanel.update();
    hornLED.update();
    if (boopInitialized) {
        booping();
    }
    renderFace();
}

void KMMXController::renderFace() {
    display.flipDMABuffer();
    delay(1000 / display.getRefreshRate());
    display.clearScreen();\
    display.drawNose(noseNew);
    mouthState.update();
    eyeState.update();
    fxState.update();
}
