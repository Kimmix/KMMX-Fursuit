#pragma once
#include <ArduinoBLE.h>
#include "Controller.h"
#include "config.h"

class BLEManager {
   public:
    static BLEManager& getInstance(Controller& controller);
    void setup();
    void poll();

   private:
    BLEManager(Controller& controller);
    Controller& controller;
    BLEService protoService;
    BLEByteCharacteristic displayBrightnessCharacteristic;
    BLEByteCharacteristic eyeStateCharacteristic;
    BLEByteCharacteristic visemeCharacteristic;

    static BLEManager* instance;

    static void blePeripheralConnectHandler(BLEDevice central);
    static void blePeripheralDisconnectHandler(BLEDevice central);
    static void displayBrightnessWritten(BLEDevice central, BLECharacteristic characteristic);
    static void eyeStateWritten(BLEDevice central, BLECharacteristic characteristic);
    static void visemeStateWritten(BLEDevice central, BLECharacteristic characteristic);
};
