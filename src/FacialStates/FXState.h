#include "RenderFunction/flyingHeart.h"

class FXState {
   private:
    LEDMatrixDisplay* display;
    FXStateEnum currentState = FXStateEnum::IDLE;
    FlyingHeart flyingHeart = FlyingHeart(display);

    unsigned long resetHeart;

   public:
    FXState(LEDMatrixDisplay* displayPtr = nullptr) : display(displayPtr) {}

    void update() {
        switch (currentState) {
            case FXStateEnum::IDLE:
                break;
            case FXStateEnum::Heart:
                flyingHeartState();
                break;
            case FXStateEnum::Blush:
                break;
            default:
                currentState = FXStateEnum::IDLE;
                break;
        }
    }

    void setState(FXStateEnum newState) {
        if (newState == FXStateEnum::Heart) {
            resetHeart = millis();
            flyingHeart.reset();
        }
        currentState = newState;
    }

    FXStateEnum getState() const {
        return currentState;
    }

    void flyingHeartState() {
        flyingHeart.renderHeart();
        if (millis() - resetHeart >= 2000) {
            currentState = FXStateEnum::IDLE;
        }
    }

    void setFlyingSpeed(int i) {
        flyingHeart.setSpeed(i);
    }
};
