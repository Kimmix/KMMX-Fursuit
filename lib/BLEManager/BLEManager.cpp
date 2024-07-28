#include "BLEManager.h"
#include <Arduino.h>

BLEManager* BLEManager::instance = nullptr;

BLEManager& BLEManager::getInstance(Controller& ctrl) {
    if (!instance) {
        instance = new BLEManager(ctrl);
    }
    return *instance;
}

BLEManager::BLEManager(Controller& ctrl) : controller(ctrl),
                                           protoService(BLE_SERVICE_UUID),
                                           displayBrightnessCharacteristic(BLE_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUID, BLERead | BLEWrite),
                                           eyeStateCharacteristic(BLE_EYE_STATE_CHARACTERISTIC_UUID, BLERead | BLEWrite),
                                           visemeCharacteristic(BLE_VISEME_CHARACTERISTIC_UUID, BLERead | BLEWrite) {
}

void BLEManager::setup() {
    Serial.println(F("Booting BLE..."));
    pinMode(LED_BUILTIN, OUTPUT);
    if (!BLE.begin()) {
        Serial.println(F("--------- Failed to initialize BLE! ---------"));
        return;
    }
    BLE.setDeviceName(BLE_DEVICE_NAME);
    BLE.setLocalName(BLE_LOCAL_NAME);

    // Define the BLE protoService and characteristic
    BLE.setAdvertisedService(protoService);
    protoService.addCharacteristic(displayBrightnessCharacteristic);
    protoService.addCharacteristic(eyeStateCharacteristic);
    protoService.addCharacteristic(visemeCharacteristic);

    // Set default values for each characteristic
    displayBrightnessCharacteristic.setValue(controller.getDisplayBrightness());
    eyeStateCharacteristic.setValue(0x00);
    visemeCharacteristic.setValue(controller.getViseme());

    BLE.addService(protoService);
    BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
    BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

    // Assign event handlers for characteristic
    displayBrightnessCharacteristic.setEventHandler(BLEWritten, displayBrightnessWritten);
    eyeStateCharacteristic.setEventHandler(BLEWritten, eyeStateWritten);
    visemeCharacteristic.setEventHandler(BLEWritten, visemeStateWritten);

    // Start advertising the BLE pService
    BLE.advertise();
    Serial.println(F("Bluetooth® device active, waiting for connections..."));
}

void BLEManager::poll() {
    BLE.poll();
}

void BLEManager::blePeripheralConnectHandler(BLEDevice central) {
    Serial.print(F("Connected event, central: "));
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH);
}

void BLEManager::blePeripheralDisconnectHandler(BLEDevice central) {
    Serial.print(F("Disconnected event, central: "));
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, LOW);
}

void BLEManager::displayBrightnessWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        instance->controller.setDisplayBrightness(static_cast<int>(*data));
    }
}

void BLEManager::eyeStateWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        instance->controller.setEye(static_cast<int>(*data));
    }
}

void BLEManager::visemeStateWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        instance->controller.setViseme(static_cast<int>(*data));
    }
}
