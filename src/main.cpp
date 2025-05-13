#include <Arduino.h>
#include "config.h"
#include "KMMXController/KMMXController.h"
#include "Network/BLE.h"

KMMXController controller;
BLEManager& bleManager = BLEManager::getInstance(controller);

void setup() {
    Wire.begin(SDA, SCL);
    Serial.begin(115200);
    if (psramInit()) {
        Serial.println("PSRAM initialized successfully");
        Serial.printf("Total heap: %d\n", ESP.getHeapSize());
        Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
        Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
        Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());
    } else {
        Serial.println("PSRAM initialization failed");
    }
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
