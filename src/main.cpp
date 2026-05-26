#include <Arduino.h>
#include "config.h"
#include "KMMXController/KMMXController.h"
#include "Network/BLE.h"
#include "Utils/Utils.h"

KMMXController controller;
BLEManager& bleManager = BLEManager::getInstance(controller);

void setup() {
    #ifndef DISABLE_SERIAL_LOGGING
    Serial.begin(115200);
    #endif

    controller.setupSensors();
    bleManager.setup();
}

void loop() {
    controller.update();
}
