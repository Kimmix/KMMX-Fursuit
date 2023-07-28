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
    EyeState eyeState = EyeState(&display);
    MouthState mouthState = MouthState(&display);

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
    }

    void setEye() {
        Serial.print("Update EYE State");
    }

    void resetFace() {
        eyeState.setState(EyeStateEnum::IDLE);
        mouthState.setState(MouthStateEnum::IDLE);
    }

    void faceBoop() {
        eyeState.setState(EyeStateEnum::BOOP);
        mouthState.setState(MouthStateEnum::BOOP);
        lastBoopTime = millis();
    }
};