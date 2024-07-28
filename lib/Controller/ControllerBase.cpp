#include "Controller.h"
#include "ControllerSensorSetup.cpp"
#include "ControllerSensorReading.cpp"
#include "ControllerRendering.cpp"
#include "ControllerBooping.cpp"
#include "ControllerStateManagement.cpp"

void Controller::setupSensors() {
    accSensor.setUp();
    proxSensor.setup();
    xTaskCreatePinnedToCore(readSensor, "SensorEventTask", 4096, this, 1, &sensorEventTaskHandle, 0);
}

void Controller::update() {
    sideLED.animate();
    booping();
    if (vsync) {
        if (millis() >= nextFrame) {
            nextFrame = millis() + frametime;
            renderFace();
        }
    } else {
        renderFace();
    }
}

void Controller::setEye(int i) {
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

void Controller::setViseme(int b) {
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

int Controller::getViseme() {
    return mouthState.getState() == MouthStateEnum::TALKING;
}

int Controller::getDisplayBrightness() {
    return display.getBrightnessValue();
}

void Controller::setDisplayBrightness(int i) {
    display.setBrightnessValue(i);
}

void Controller::recieveEspNow(int16_t data) {
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

void Controller::updatePixelPosition(int16_t y) {
    Serial.println(y, BIN);
    pixelPos = y;
}

void Controller::debugPixel(int16_t p) {
    int bit_position = 0;
    while (p > 0) {
        if (p & 1) {
            display.drawPixel(0, bit_position, display.color565(0, 200, 255));
        }
        p >>= 1;
        bit_position++;
    }
}
