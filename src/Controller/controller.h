enum class EyeStateEnum { IDLE,
                          BLINK,
                          BOOP,
                          GOOGLY };
enum class MouthStateEnum { IDLE,
                            BOOP,
                            TALKING };

#include "Devices/LEDMatrixDisplay.h"
#include "Devices/SideLED.h"
#include "FacialStates/FacialState.h"
#include "FacialStates/MouthState.h"
#include "FacialStates/EyeState.h"
#include "Bitmaps/Icons.h"

#define IR_PIN GPIO_NUM_35
#define RANDOM_PIN GPIO_NUM_36
class Controller {
   private:
    LEDMatrixDisplay display;
    SideLED sideLED;
    EyeStateEnum currentEyeState = EyeStateEnum::IDLE;
    MouthStateEnum currentMouthState = MouthStateEnum::IDLE;
    EyeState eyeState = EyeState(currentEyeState, &display);
    MouthState mouthState = MouthState(currentMouthState, &display);

    unsigned long lastBoopTime = 0;
    const unsigned long boopDuration = 500;

   public:
    void setupDevices() {
        pinMode(IR_PIN, INPUT);
        randomSeed(analogRead(RANDOM_PIN));
    }

    void update() {
        renderFace();
        // sideLED.animate();
    }

    void renderFace() {
        display.drawColorTest();
        display.drawNose(noseDefault);
        mouthState.update();
        eyeState.update();
        // Double Buffering
        display.render();
        display.clearScreen();
        resetBoop();
    }

    void setEye() {
        Serial.print("Update EYE State");
    }

    void resetFace() {
        currentEyeState = EyeStateEnum::IDLE;
        currentMouthState = MouthStateEnum::IDLE;
    }

    void faceBoop() {
        currentMouthState = MouthStateEnum::BOOP;
        currentEyeState = EyeStateEnum::BOOP;
        lastBoopTime = millis();
    }

    void resetBoop() {
        if (currentEyeState == EyeStateEnum::BOOP && millis() - lastBoopTime >= boopDuration) {
            resetFace();
        }
    }
};