#include "KMMXController.h"

// Brightness Control
int KMMXController::getDisplayBrightness() {
    return display.getBrightnessValue();
}

void KMMXController::setDisplayBrightness(int i) {
    display.setBrightnessValue(i);
}

// State control for BLE
void KMMXController::setEye(int i) {
    switch (i) {
        case 1:
            eyeState.setState(EyeStateEnum::GOOGLY);
            break;
        case 2:
            eyeState.setState(EyeStateEnum::HEART);
            break;
        case 3:
            eyeState.setState(EyeStateEnum::SMILE);
            break;
        default:
            eyeState.setState(EyeStateEnum::IDLE);
            break;
    }
}

void KMMXController::setViseme(int b) {
    static const int noiseThresholds[] = {400, 600, 1000, 1600, 3200, 6400, 1200, 18000, 25000};
    static const int numThresholds = sizeof(noiseThresholds) / sizeof(noiseThresholds[0]);
    if (b == 0) {
        mouthState.setState(MouthStateEnum::IDLE);
    } else if (b == 1 || b > numThresholds) {
        mouthState.setState(MouthStateEnum::TALKING);
    } else {
        mouthState.viseme.setNoiseThreshold(noiseThresholds[b - 2]);
    }
}

int KMMXController::getViseme() {
    return mouthState.getState() == MouthStateEnum::TALKING;
}

// ESP-NOW! - Unused!!
void KMMXController::recieveEspNow(int16_t data) {
    switch (data) {
        case 1:
            eyeState.setState(EyeStateEnum::IDLE);
            break;
        case 2:
            eyeState.setState(EyeStateEnum::SMILE);
            break;
        case 3:
            eyeState.setState(EyeStateEnum::OEYE);
            break;
        default:
            break;
    }
}

void KMMXController::updatePixelPosition(int16_t y) {
    Serial.println(y, BIN);
    pixelPos = y;
}

void KMMXController::debugPixel(int16_t p) {
    int bit_position = 0;
    while (p > 0) {
        if (p & 1) {
            display.drawPixel(0, bit_position, display.color565(0, 200, 255));
        }
        p >>= 1;
        bit_position++;
    }
}
