#include <Arduino.h>
#include <esp_random.h>

#include "Bitmaps/Icons.h"
#include "Devices/LEDMatrixDisplay.h"
#include "Devices/LIS3DH.h"
#include "Devices/Bluetooth.h"
#include "Devices/I2SMicrophone.h"
#include "Controller/eyeController.h"
#include "Controller/mouthController.h"

#define IR_PIN GPIO_NUM_35
#define RANDOM_PIN GPIO_NUM_36
bool isBoop;

LIS3DH lis;
DisplayController display;
Microphone microphone;

EyeState eyeState(&display, &lis);
MouthState mouthState(&display, &microphone);
BluetoothController ble(&display);

uint16_t fps = 0;
unsigned long fps_timer;
void showFPS() {
    ++fps;
    if (millis() > fps_timer) {
        Serial.printf_P(PSTR("Effect fps: %d\n"), fps);
        fps_timer = millis() + 1000;
        fps = 0;
    }
}

TaskHandle_t controlMouth;
void asyncRender(void* parameter) {
    while (true) {
        if (isBoop) {
            mouthState.setBoop();
        }
        mouthState.update();
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(100);
    microphone.init();
    lis.init();
    // ble.init();
    pinMode(IR_PIN, INPUT);
    randomSeed(analogRead(RANDOM_PIN));
    xTaskCreatePinnedToCore(asyncRender, "Render Mouth", 10000, NULL, 0, &controlMouth, 0);
}

void loop() {
    // ble.update();
    // display.render();
    // display.drawColorTest();
    display.drawNose(noseDefault);
    isBoop = !digitalRead(IR_PIN);
    if (isBoop) {
        eyeState.setBoop();
    }
    eyeState.update();
    // showFPS();
}