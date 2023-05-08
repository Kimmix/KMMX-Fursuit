#include <Arduino.h>
#include <esp_random.h>

#include "Controller/controller.h"

#define IR_PIN GPIO_NUM_35
#define RANDOM_PIN GPIO_NUM_36
bool isBoop;

Controller controller;

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

// TaskHandle_t controlMouth;
// void asyncRender(void* parameter) {
//     while (true) {
//         if (isBoop) {
//             mouthState.setBoop();
//         }
//         mouthState.update();
//     }
// }

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(400);
    // ble.init();
    pinMode(IR_PIN, INPUT);
    randomSeed(analogRead(RANDOM_PIN));
    // xTaskCreatePinnedToCore(asyncRender, "Render Mouth", 10000, NULL, 0, &controlMouth, 0);
}

void loop() {
    // showFPS();
    isBoop = !digitalRead(IR_PIN);
    if (isBoop) {
        controller.faceBoop();
    }
    controller.render();
}