#include "Bitmaps/mouthBitmap.h"
#include "RenderFunction/viseme.h"

class MouthState {
   public:
    MouthState(DisplayController* displayPtr = nullptr) : display(displayPtr),
                                                          currentState(IDLE) {}

    void update() {
        // Serial.print(currentState);
        // Serial.print("\n");
        switch (currentState) {
            case IDLE:
                display->drawMouth(mouthDefault);
                break;
            case BOOP:
                display->drawMouth(mouthOpen);
                currentState = TALKING;
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
    }

   private:
    DisplayController* display;
    Viseme viseme;

    enum State {
        IDLE,
        BOOP,
        TALKING,
    };
    State currentState;
};
