#pragma once
#include <NimBLEDevice.h>
#include "KMMXController/KMMXController.h"

// Callback classes for NimBLE characteristics
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
   public:
    CharacteristicCallbacks(void (*callback)(const uint8_t*, size_t)) : onWriteCallback(callback) {}
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
        if (onWriteCallback) {
            onWriteCallback(pCharacteristic->getValue().data(), pCharacteristic->getValue().length());
        }
    }

   private:
    void (*onWriteCallback)(const uint8_t*, size_t);
};

class ServerCallbacks : public NimBLEServerCallbacks {
   public:
    ServerCallbacks(void (*onConnectCallback)(), void (*onDisconnectCallback)())
        : onConnectCb(onConnectCallback), onDisconnectCb(onDisconnectCallback) {}

    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
        if (onConnectCb) onConnectCb();
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
        if (onDisconnectCb) onDisconnectCb();
    }

   private:
    void (*onConnectCb)();
    void (*onDisconnectCb)();
};

class BLEManager {
   public:
    static BLEManager& getInstance(KMMXController& controller);
    void setup();
    void poll();
    bool isConnected() const;

   private:
    BLEManager(KMMXController& controller);
    KMMXController& controller;

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

    static BLEManager* instance;

    // Callback handlers
    static void onConnect();
    static void onDisconnect();
    static void onDisplayBrightnessWrite(const uint8_t* data, size_t length);
    static void onEyeStateWrite(const uint8_t* data, size_t length);
    static void onMouthStateWrite(const uint8_t* data, size_t length);
    static void onVisemeStateWrite(const uint8_t* data, size_t length);
    static void onHornBrightnessWrite(const uint8_t* data, size_t length);
    static void onCheekBrightnessWrite(const uint8_t* data, size_t length);
    static void onCheekBgColorWrite(const uint8_t* data, size_t length);
    static void onCheekFadeColorWrite(const uint8_t* data, size_t length);
    static void onDisplayColorModeWrite(const uint8_t* data, size_t length);
    static void onDisplayEffectColor1Write(const uint8_t* data, size_t length);
    static void onDisplayEffectColor2Write(const uint8_t* data, size_t length);
    static void onDisplayEffectOption1Write(const uint8_t* data, size_t length);
    static void onDisplayEffectOption2Write(const uint8_t* data, size_t length);
    static void onDisplayEffectOption3Write(const uint8_t* data, size_t length);
    static void onRebootWrite(const uint8_t* data, size_t length);
};
