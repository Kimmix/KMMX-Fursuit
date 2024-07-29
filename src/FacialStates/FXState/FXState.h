#pragma once
#include "Devices/LEDMatrixDisplay/Hub75DMA.h"
#include "Renderer/FlyingHeart.h"

enum class FXStateEnum { IDLE, Heart, Blush };

class FXState {
   private:
    Hub75DMA* display;
    FXStateEnum currentState = FXStateEnum::IDLE;
    FlyingHeart flyingHeart;

    unsigned long resetHeart;

   public:
    FXState(Hub75DMA* displayPtr = nullptr);

    void update();
    void setState(FXStateEnum newState);
    FXStateEnum getState() const;
    void flyingHeartState();
    void setFlyingSpeed(float i);
};
