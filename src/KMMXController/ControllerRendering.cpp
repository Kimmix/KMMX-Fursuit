#include "KMMXController.h"

void KMMXController::update() {
    // Update devices that are present on this board
    #if HAS_CHEEK_PANEL
    statusLED.update();
    cheekPanel.update();
    #endif

    #if HAS_HORN_LED
    hornLED.update();
    #endif

    // Handle boop interaction if proximity sensor is available
    #if HAS_PROXIMITY
    if (boopInitialized) {
        handleBoop();
    }
    #endif
}

void KMMXController::renderFace() {
    display.flipDMABuffer();
    display.clearScreen();
    display.updateColorEffectsFrame();  // Update cached time once per frame
    display.drawNose(noseNew);
    mouthState.update();
    eyeState.update();
    fxState.update();
}

void KMMXController::renderTask(void *parameter) {
    KMMXController *ctrl = static_cast<KMMXController *>(parameter);

    // Calculate frame interval based on refresh rate
    int refreshRate = ctrl->display.getRefreshRate();
    int frameDelayMs = 1000 / refreshRate;

    while (true) {
        // Flip buffer to display the previously rendered frame
        ctrl->display.flipDMABuffer();

        // Wait for DMA to complete the buffer swap (this is critical!)
        vTaskDelay(pdMS_TO_TICKS(frameDelayMs));

        // Now render the next frame to the back buffer
        ctrl->display.clearScreen();
        ctrl->display.updateColorEffectsFrame();  // Update cached time once per frame
        ctrl->display.drawNose(noseNew);
        ctrl->mouthState.update();
        ctrl->eyeState.update();
        ctrl->fxState.update();
    }
}
