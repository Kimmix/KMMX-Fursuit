#pragma once
#include <ArduinoBLE.h>
#include "KMMXController/KMMXController.h"

class BLEManager {
   public:
    static BLEManager& getInstance(KMMXController& controller);
    void setup();
    void poll();
    bool isConnected() const;

   private:
    BLEManager(KMMXController& controller);
    KMMXController& controller;
    BLEService protoService;
    BLEByteCharacteristic displayBrightnessCharacteristic;
    BLEByteCharacteristic eyeStateCharacteristic;
    BLEByteCharacteristic mouthStateCharacteristic;
    BLEByteCharacteristic visemeCharacteristic;
    BLEByteCharacteristic hornBrightnessCharacteristic;
    BLEByteCharacteristic cheekBrightnessCharacteristic;
    BLECharacteristic cheekBgColorCharacteristic;
    BLECharacteristic cheekFadeColorCharacteristic;
    BLEByteCharacteristic displayColorModeCharacteristic;
    BLECharacteristic displayEffectColor1Characteristic;
    BLECharacteristic displayEffectColor2Characteristic;
    BLEByteCharacteristic displayEffectOption1Characteristic;
    BLEByteCharacteristic displayEffectOption2Characteristic;
    BLEByteCharacteristic displayEffectOption3Characteristic;
    BLEByteCharacteristic rebootCharacteristic;

    static BLEManager* instance;

    static void blePeripheralConnectHandler(BLEDevice central);
    static void blePeripheralDisconnectHandler(BLEDevice central);
    static void displayBrightnessWritten(BLEDevice central, BLECharacteristic characteristic);
    static void eyeStateWritten(BLEDevice central, BLECharacteristic characteristic);
    static void mouthStateWritten(BLEDevice central, BLECharacteristic characteristic);
    static void visemeStateWritten(BLEDevice central, BLECharacteristic characteristic);
    static void hornBrightnessWritten(BLEDevice central, BLECharacteristic characteristic);
    static void cheekBrightnessWritten(BLEDevice central, BLECharacteristic characteristic);
    static void cheekBgColorWritten(BLEDevice central, BLECharacteristic characteristic);
    static void cheekFadeColorWritten(BLEDevice central, BLECharacteristic characteristic);
    static void displayColorModeWritten(BLEDevice central, BLECharacteristic characteristic);
    static void displayEffectColor1Written(BLEDevice central, BLECharacteristic characteristic);
    static void displayEffectColor2Written(BLEDevice central, BLECharacteristic characteristic);
    static void displayEffectOption1Written(BLEDevice central, BLECharacteristic characteristic);
    static void displayEffectOption2Written(BLEDevice central, BLECharacteristic characteristic);
    static void displayEffectOption3Written(BLEDevice central, BLECharacteristic characteristic);
    static void rebootWritten(BLEDevice central, BLECharacteristic characteristic);
};
