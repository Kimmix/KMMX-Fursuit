#pragma once
#include "LEDMatrixDisplay.h"
#include "FlyingHeart.h"

enum class FXStateEnum { IDLE, Heart, Blush };

class FXState {
   private:
    LEDMatrixDisplay* display;
    FXStateEnum currentState = FXStateEnum::IDLE;
    FlyingHeart flyingHeart;

    unsigned long resetHeart;

   public:
    FXState(LEDMatrixDisplay* displayPtr = nullptr);

    void update();
    void setState(FXStateEnum newState);
    FXStateEnum getState() const;
    void flyingHeartState();
    void setFlyingSpeed(float i);
};
