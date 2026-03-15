#include "BLE.h"
#include <Arduino.h>
#include "config.h"


BLEManager* BLEManager::instance = nullptr;

BLEManager& BLEManager::getInstance(KMMXController& ctrl) {
    if (!instance) {
        instance = new BLEManager(ctrl);
    }
    return *instance;
}

BLEManager::BLEManager(KMMXController& ctrl) : controller(ctrl),
                                           protoService(BLE_SERVICE_UUID),
                                           displayBrightnessCharacteristic(BLE_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUID, BLERead | BLEWrite),
                                           eyeStateCharacteristic(BLE_EYE_STATE_CHARACTERISTIC_UUID, BLERead | BLEWrite),
                                           visemeCharacteristic(BLE_VISEME_CHARACTERISTIC_UUID, BLERead | BLEWrite),
                                           hornBrightnessCharacteristic(BLE_HORN_BRIGHTNESS_CHARACTERISTIC_UUID, BLERead | BLEWrite),
                                           cheekBrightnessCharacteristic(BLE_CHEEK_BRIGHTNESS_CHARACTERISTIC_UUID, BLERead | BLEWrite),
                                           cheekBgColorCharacteristic(BLE_CHEEK_BG_COLOR_CHARACTERISTIC_UUID, BLERead | BLEWrite, 3),
                                           cheekFadeColorCharacteristic(BLE_CHEEK_FADE_COLOR_CHARACTERISTIC_UUID, BLERead | BLEWrite, 3) {
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
    protoService.addCharacteristic(hornBrightnessCharacteristic);
    protoService.addCharacteristic(cheekBrightnessCharacteristic);
    protoService.addCharacteristic(cheekBgColorCharacteristic);
    protoService.addCharacteristic(cheekFadeColorCharacteristic);

    // Set default values for each characteristic
    displayBrightnessCharacteristic.setValue(controller.getDisplayBrightness());
    eyeStateCharacteristic.setValue(0x00);
    visemeCharacteristic.setValue(controller.getViseme());
    hornBrightnessCharacteristic.setValue(controller.getHornBrightness());
    cheekBrightnessCharacteristic.setValue(controller.getCheekBrightness());

    // Set default color values (RGB format)
    uint32_t bgColor = controller.getCheekBackgroundColor();
    uint8_t bgColorData[3] = {(uint8_t)(bgColor >> 16), (uint8_t)(bgColor >> 8), (uint8_t)bgColor};
    cheekBgColorCharacteristic.setValue(bgColorData, 3);

    uint32_t fadeColor = controller.getCheekFadeColor();
    uint8_t fadeColorData[3] = {(uint8_t)(fadeColor >> 16), (uint8_t)(fadeColor >> 8), (uint8_t)fadeColor};
    cheekFadeColorCharacteristic.setValue(fadeColorData, 3);

    BLE.addService(protoService);
    BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
    BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

    // Assign event handlers for characteristic
    displayBrightnessCharacteristic.setEventHandler(BLEWritten, displayBrightnessWritten);
    eyeStateCharacteristic.setEventHandler(BLEWritten, eyeStateWritten);
    visemeCharacteristic.setEventHandler(BLEWritten, visemeStateWritten);
    hornBrightnessCharacteristic.setEventHandler(BLEWritten, hornBrightnessWritten);
    cheekBrightnessCharacteristic.setEventHandler(BLEWritten, cheekBrightnessWritten);
    cheekBgColorCharacteristic.setEventHandler(BLEWritten, cheekBgColorWritten);
    cheekFadeColorCharacteristic.setEventHandler(BLEWritten, cheekFadeColorWritten);

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

void BLEManager::hornBrightnessWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        instance->controller.setHornBrightness(static_cast<int>(*data));
    }
}

void BLEManager::cheekBrightnessWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        instance->controller.setCheekBrightness(static_cast<int>(*data));
    }
}

void BLEManager::cheekBgColorWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        if (characteristic.valueLength() >= 3) {
            instance->controller.setCheekBackgroundColor(data[0], data[1], data[2]);
        }
    }
}

void BLEManager::cheekFadeColorWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        if (characteristic.valueLength() >= 3) {
            instance->controller.setCheekFadeColor(data[0], data[1], data[2]);
        }
    }
}
