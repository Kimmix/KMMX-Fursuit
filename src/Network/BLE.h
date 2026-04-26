#pragma once
#include <NimBLEDevice.h>
#include "KMMXController/KMMXController.h"

// Forward declaration
class BLEManager;

// Forward declare all callback classes
class ServerCallbacks;
class DisplayBrightnessCallbacks;
class EyeStateCallbacks;
class MouthStateCallbacks;
class VisemeCallbacks;
class HornBrightnessCallbacks;
class CheekBrightnessCallbacks;
class CheekBgColorCallbacks;
class CheekFadeColorCallbacks;
class DisplayColorModeCallbacks;
class DisplayEffectColor1Callbacks;
class DisplayEffectColor2Callbacks;
class DisplayEffectOption1Callbacks;
class DisplayEffectOption2Callbacks;
class DisplayEffectOption3Callbacks;
class RebootCallbacks;

class BLEManager {
   public:
    static BLEManager& getInstance(KMMXController& controller);
    void setup();
    bool isConnected() const;

    static BLEManager* instance;  // Made public for callbacks to access

    // Allow all callback classes to access private members
    friend class ServerCallbacks;
    friend class DisplayBrightnessCallbacks;
    friend class EyeStateCallbacks;
    friend class MouthStateCallbacks;
    friend class VisemeCallbacks;
    friend class HornBrightnessCallbacks;
    friend class CheekBrightnessCallbacks;
    friend class CheekBgColorCallbacks;
    friend class CheekFadeColorCallbacks;
    friend class DisplayColorModeCallbacks;
    friend class DisplayEffectColor1Callbacks;
    friend class DisplayEffectColor2Callbacks;
    friend class DisplayEffectOption1Callbacks;
    friend class DisplayEffectOption2Callbacks;
    friend class DisplayEffectOption3Callbacks;
    friend class RebootCallbacks;

   private:
    BLEManager(KMMXController& controller);
    KMMXController& controller;

    bool debugEnabled = true;  // Flag to control debug output

    NimBLEServer* pServer;
    NimBLEService* pService;
    NimBLECharacteristic* displayBrightnessCharacteristic;
    NimBLECharacteristic* eyeStateCharacteristic;
    NimBLECharacteristic* mouthStateCharacteristic;
    NimBLECharacteristic* visemeCharacteristic;
    NimBLECharacteristic* hornBrightnessCharacteristic;
    NimBLECharacteristic* cheekBrightnessCharacteristic;
    NimBLECharacteristic* cheekBgColorCharacteristic;
    NimBLECharacteristic* cheekFadeColorCharacteristic;
    NimBLECharacteristic* displayColorModeCharacteristic;
    NimBLECharacteristic* displayEffectColor1Characteristic;
    NimBLECharacteristic* displayEffectColor2Characteristic;
    NimBLECharacteristic* displayEffectOption1Characteristic;
    NimBLECharacteristic* displayEffectOption2Characteristic;
    NimBLECharacteristic* displayEffectOption3Characteristic;
    NimBLECharacteristic* rebootCharacteristic;
};
