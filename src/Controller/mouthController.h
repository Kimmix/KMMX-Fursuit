#include "Bitmaps/mouthBitmap.h"
#include "RenderFunction/viseme.h"

#define AH_MIN 600
#define AH_MAX 1200
#define EE_MIN 1000
#define EE_MAX 2000
#define OH_MIN 1800
#define OH_MAX 2800
#define OO_MIN 2600
#define OO_MAX 3600
#define TH_MIN 3400
#define TH_MAX 4000

#define SAMPLE_RATE 8000
#define SAMPLES 256

#define NOISE_THRESHOLD 300

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
                talking();
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

    void talking() {
        display->drawMouth(viseme.renderViseme());
    }
};
