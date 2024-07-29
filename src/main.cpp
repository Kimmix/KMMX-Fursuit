#include <Arduino.h>
#include "config.h"
#include "KMMXController/KMMXController.h"
#include "Network/BLE.h"

KMMXController controller;
BLEManager& bleManager = BLEManager::getInstance(controller);

void setup() {
    Wire.begin(S3_SDA, S3_SCL);
    Serial.begin(115200);
    randomSeed(analogRead(RANDOM_PIN));
    controller.setupSensors();
    bleManager.setup();
    // setupEspNow();
}

void loop() {
    bleManager.poll();
    controller.update();
    // showFPS();
}
