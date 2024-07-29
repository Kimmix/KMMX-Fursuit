#include "HornLED.h"

HornLED::HornLED() {
    ledcSetup(hornPwmChannel, hornFrequency, hornResolution);
    ledcAttachPin(hornPin, hornPwmChannel);
    ledcWrite(hornPwmChannel, brightness);
}

void HornLED::setBrightness(unsigned short value) {
    brightness = map(value, 0, 100, hornMinBrightness, hornMaxBrightness);
    brightness = constrain(brightness, hornMinBrightness, hornMaxBrightness);
    ledcWrite(hornPwmChannel, brightness);
}

unsigned short HornLED::getBrightness() const {
    return brightness;
}
