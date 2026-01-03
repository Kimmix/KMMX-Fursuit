#include <Arduino.h>
#include "config.h"
#include "KMMXController/KMMXController.h"
#include "Network/BLE.h"
#include "Utils/Utils.h"

KMMXController controller;
BLEManager& bleManager = BLEManager::getInstance(controller);

void setup() {
    Serial.begin(115200);
    randomSeed(analogRead(RANDOM_PIN));
    controller.setupSensors();
    bleManager.setup();
}

void loop() {
    bleManager.poll();
    controller.update();
    // showFPS();
}
