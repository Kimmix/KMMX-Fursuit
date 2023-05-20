#include "Bitmaps/mouthBitmap.h"
#include "RenderFunction/viseme.h"

class MouthState {
   public:
    MouthState(LEDMatrixDisplay* displayPtr = nullptr) : display(displayPtr) {}

    void update() {
        // Serial.print(currentState);
        // Serial.print("\n");
        switch (currentState) {
            case IDLE:
                display->drawMouth(mouthDefault);
                break;
            case BOOP:
                display->drawMouth(mouthOpen);
                if (millis() - resetBoop >= 100) {
                    currentState = TALKING;
                }
                // currentState = TALKING;
                break;
            case TALKING:
                display->drawMouth(viseme.renderViseme());
                break;
            default:
                break;
        }
    }

    void setIdle() {
        currentState = IDLE;
    }
    void setTalk() {
        currentState = TALKING;
    }
    void setBoop() {
        currentState = BOOP;
        resetBoop = millis();
    }

   private:
    LEDMatrixDisplay* display;
    Viseme viseme;

    enum State {
        IDLE,
        BOOP,
        TALKING,
    };
    State currentState = IDLE;
    unsigned long resetBoop;
};
