#include "KMMXController.h"

// Brightness Control
int KMMXController::getDisplayBrightness() {
    return display.getBrightnessValue();
}

void KMMXController::setDisplayBrightness(int i) {
    display.setBrightnessValue(i);
}

int KMMXController::getHornBrightness() {
    return hornLED.getBrightness();
}

void KMMXController::setHornBrightness(int i) {
    hornLED.setBrightness(i, 20);  // Set brightness with fade speed of 20 for faster response
}

int KMMXController::getCheekBrightness() {
    return cheekPanel.getBrightness();
}

void KMMXController::setCheekBrightness(int i) {
    cheekPanel.setBrightness(constrain(i, 0, 255));
}

void KMMXController::setCheekBackgroundColor(uint8_t r, uint8_t g, uint8_t b) {
    cheekPanel.setBackgroundColorRGB(r, g, b);
}

void KMMXController::setCheekFadeColor(uint8_t r, uint8_t g, uint8_t b) {
    cheekPanel.setFadeColorRGB(r, g, b);
}

uint32_t KMMXController::getCheekBackgroundColor() {
    return cheekPanel.getBackgroundColor();
}

uint32_t KMMXController::getCheekFadeColor() {
    return cheekPanel.getFadeColor();
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
        case 4:
            eyeState.setState(EyeStateEnum::ANGRY);
            break;
        case 5:
            eyeState.setState(EyeStateEnum::SAD);
            break;
        case 6:
            eyeState.setState(EyeStateEnum::BOOP);
            break;
        case 7:
            eyeState.setState(EyeStateEnum::OEYE);
            break;
        case 8:
            eyeState.setState(EyeStateEnum::CRY);
            break;
        case 9:
            eyeState.setState(EyeStateEnum::DOUBTED);
            break;
        case 10:
            eyeState.setState(EyeStateEnum::ROUNDED);
            break;
        case 11:
            eyeState.setState(EyeStateEnum::SHARP);
            break;
        case 12:
            eyeState.setState(EyeStateEnum::GIGGLE);
            break;
        case 13:
            eyeState.setState(EyeStateEnum::UNIMPRESSED);
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

// System Control
void KMMXController::reboot() {
    Serial.println("Rebooting device...");
    delay(100);  // Give time for serial message to be sent
    ESP.restart();
}
