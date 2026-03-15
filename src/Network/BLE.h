#pragma once
#include <ArduinoBLE.h>
#include "KMMXController/KMMXController.h"

class BLEManager {
   public:
    static BLEManager& getInstance(KMMXController& controller);
    void setup();
    void poll();

   private:
    BLEManager(KMMXController& controller);
    KMMXController& controller;
    BLEService protoService;
    BLEByteCharacteristic displayBrightnessCharacteristic;
    BLEByteCharacteristic eyeStateCharacteristic;
    BLEByteCharacteristic visemeCharacteristic;
    BLEByteCharacteristic hornBrightnessCharacteristic;
    BLEByteCharacteristic cheekBrightnessCharacteristic;
    BLECharacteristic cheekBgColorCharacteristic;
    BLECharacteristic cheekFadeColorCharacteristic;
    BLEByteCharacteristic rebootCharacteristic;

    static BLEManager* instance;

    static void blePeripheralConnectHandler(BLEDevice central);
    static void blePeripheralDisconnectHandler(BLEDevice central);
    static void displayBrightnessWritten(BLEDevice central, BLECharacteristic characteristic);
    static void eyeStateWritten(BLEDevice central, BLECharacteristic characteristic);
    static void visemeStateWritten(BLEDevice central, BLECharacteristic characteristic);
    static void hornBrightnessWritten(BLEDevice central, BLECharacteristic characteristic);
    static void cheekBrightnessWritten(BLEDevice central, BLECharacteristic characteristic);
    static void cheekBgColorWritten(BLEDevice central, BLECharacteristic characteristic);
    static void cheekFadeColorWritten(BLEDevice central, BLECharacteristic characteristic);
    static void rebootWritten(BLEDevice central, BLECharacteristic characteristic);
};
