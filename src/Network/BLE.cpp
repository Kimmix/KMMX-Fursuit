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
                                           mouthStateCharacteristic(BLE_MOUTH_STATE_CHARACTERISTIC_UUID, BLERead | BLEWrite),
                                           visemeCharacteristic(BLE_VISEME_CHARACTERISTIC_UUID, BLERead | BLEWrite),
                                           hornBrightnessCharacteristic(BLE_HORN_BRIGHTNESS_CHARACTERISTIC_UUID, BLERead | BLEWrite),
                                           cheekBrightnessCharacteristic(BLE_CHEEK_BRIGHTNESS_CHARACTERISTIC_UUID, BLERead | BLEWrite),
                                           cheekBgColorCharacteristic(BLE_CHEEK_BG_COLOR_CHARACTERISTIC_UUID, BLERead | BLEWrite, 3),
                                           cheekFadeColorCharacteristic(BLE_CHEEK_FADE_COLOR_CHARACTERISTIC_UUID, BLERead | BLEWrite, 3),
                                           displayColorModeCharacteristic(BLE_DISPLAY_COLOR_MODE_CHARACTERISTIC_UUID, BLERead | BLEWrite),
                                           displayEffectColor1Characteristic(BLE_DISPLAY_EFFECT_COLOR1_CHARACTERISTIC_UUID, BLERead | BLEWrite, 3),
                                           displayEffectColor2Characteristic(BLE_DISPLAY_EFFECT_COLOR2_CHARACTERISTIC_UUID, BLERead | BLEWrite, 3),
                                           displayEffectOption1Characteristic(BLE_DISPLAY_EFFECT_OPTION1_CHARACTERISTIC_UUID, BLERead | BLEWrite),
                                           displayEffectOption2Characteristic(BLE_DISPLAY_EFFECT_OPTION2_CHARACTERISTIC_UUID, BLERead | BLEWrite),
                                           displayEffectOption3Characteristic(BLE_DISPLAY_EFFECT_OPTION3_CHARACTERISTIC_UUID, BLERead | BLEWrite),
                                           rebootCharacteristic(BLE_REBOOT_CHARACTERISTIC_UUID, BLEWrite) {
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
    protoService.addCharacteristic(mouthStateCharacteristic);
    protoService.addCharacteristic(visemeCharacteristic);
    protoService.addCharacteristic(hornBrightnessCharacteristic);
    protoService.addCharacteristic(cheekBrightnessCharacteristic);
    protoService.addCharacteristic(cheekBgColorCharacteristic);
    protoService.addCharacteristic(cheekFadeColorCharacteristic);
    protoService.addCharacteristic(displayColorModeCharacteristic);
    protoService.addCharacteristic(displayEffectColor1Characteristic);
    protoService.addCharacteristic(displayEffectColor2Characteristic);
    protoService.addCharacteristic(displayEffectOption1Characteristic);
    protoService.addCharacteristic(displayEffectOption2Characteristic);
    protoService.addCharacteristic(displayEffectOption3Characteristic);
    protoService.addCharacteristic(rebootCharacteristic);

    // Set default values for each characteristic
    displayBrightnessCharacteristic.setValue(controller.getDisplayBrightness());
    eyeStateCharacteristic.setValue(0x00);
    mouthStateCharacteristic.setValue(0x00);
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

    // Set display color mode and effect color values
    displayColorModeCharacteristic.setValue(controller.getDisplayColorMode());

    uint8_t color1R, color1G, color1B, color2R, color2G, color2B;
    controller.getDisplayGradientTopColor(color1R, color1G, color1B);
    controller.getDisplayGradientBottomColor(color2R, color2G, color2B);
    uint8_t color1Data[3] = {color1R, color1G, color1B};
    uint8_t color2Data[3] = {color2R, color2G, color2B};
    displayEffectColor1Characteristic.setValue(color1Data, 3);
    displayEffectColor2Characteristic.setValue(color2Data, 3);

    // Set effect option values (Option1 = thickness, Option2 = speed, Option3 = direction for modes 4 & 5)
    displayEffectOption1Characteristic.setValue(controller.getDisplayEffectThickness());
    displayEffectOption2Characteristic.setValue(controller.getDisplayEffectSpeed());
    displayEffectOption3Characteristic.setValue(controller.getDisplayEffectDirectionInverted());

    BLE.addService(protoService);
    BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
    BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

    // Assign event handlers for characteristic
    displayBrightnessCharacteristic.setEventHandler(BLEWritten, displayBrightnessWritten);
    eyeStateCharacteristic.setEventHandler(BLEWritten, eyeStateWritten);
    mouthStateCharacteristic.setEventHandler(BLEWritten, mouthStateWritten);
    visemeCharacteristic.setEventHandler(BLEWritten, visemeStateWritten);
    hornBrightnessCharacteristic.setEventHandler(BLEWritten, hornBrightnessWritten);
    cheekBrightnessCharacteristic.setEventHandler(BLEWritten, cheekBrightnessWritten);
    cheekBgColorCharacteristic.setEventHandler(BLEWritten, cheekBgColorWritten);
    cheekFadeColorCharacteristic.setEventHandler(BLEWritten, cheekFadeColorWritten);
    displayColorModeCharacteristic.setEventHandler(BLEWritten, displayColorModeWritten);
    displayEffectColor1Characteristic.setEventHandler(BLEWritten, displayEffectColor1Written);
    displayEffectColor2Characteristic.setEventHandler(BLEWritten, displayEffectColor2Written);
    displayEffectOption1Characteristic.setEventHandler(BLEWritten, displayEffectOption1Written);
    displayEffectOption2Characteristic.setEventHandler(BLEWritten, displayEffectOption2Written);
    displayEffectOption3Characteristic.setEventHandler(BLEWritten, displayEffectOption3Written);
    rebootCharacteristic.setEventHandler(BLEWritten, rebootWritten);

    // Start advertising the BLE pService
    BLE.advertise();
    Serial.println(F("Bluetooth® device active, waiting for connections..."));
}

void BLEManager::poll() {
    BLE.poll();
}

bool BLEManager::isConnected() const {
    return BLE.connected();
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
        Serial.print(F("[BLE] Display Brightness: "));
        Serial.println(*data);
        instance->controller.setDisplayBrightness(static_cast<int>(*data));
    }
}

void BLEManager::eyeStateWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        Serial.print(F("[BLE] Eye State: "));
        Serial.println(*data);
        instance->controller.setEye(static_cast<int>(*data));
    }
}

void BLEManager::mouthStateWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        Serial.print(F("[BLE] Mouth State: "));
        Serial.println(*data);
        instance->controller.setMouth(static_cast<int>(*data));
    }
}

void BLEManager::visemeStateWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        Serial.print(F("[BLE] Viseme: "));
        Serial.println(*data);
        instance->controller.setViseme(static_cast<int>(*data));
    }
}

void BLEManager::hornBrightnessWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        Serial.print(F("[BLE] Horn Brightness: "));
        Serial.println(*data);
        instance->controller.setHornBrightness(static_cast<int>(*data));
    }
}

void BLEManager::cheekBrightnessWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        Serial.print(F("[BLE] Cheek Brightness: "));
        Serial.println(*data);
        instance->controller.setCheekBrightness(static_cast<int>(*data));
    }
}

void BLEManager::cheekBgColorWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        if (characteristic.valueLength() >= 3) {
            Serial.print(F("[BLE] Cheek BG Color: R="));
            Serial.print(data[0]);
            Serial.print(F(" G="));
            Serial.print(data[1]);
            Serial.print(F(" B="));
            Serial.println(data[2]);
            instance->controller.setCheekBackgroundColor(data[0], data[1], data[2]);
        }
    }
}

void BLEManager::cheekFadeColorWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        if (characteristic.valueLength() >= 3) {
            Serial.print(F("[BLE] Cheek Fade Color: R="));
            Serial.print(data[0]);
            Serial.print(F(" G="));
            Serial.print(data[1]);
            Serial.print(F(" B="));
            Serial.println(data[2]);
            instance->controller.setCheekFadeColor(data[0], data[1], data[2]);
        }
    }
}

void BLEManager::displayColorModeWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        Serial.print(F("[BLE] Display Color Mode: "));
        Serial.println(*data);
        instance->controller.setDisplayColorMode(static_cast<uint8_t>(*data));
    }
}

void BLEManager::displayEffectColor1Written(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        if (characteristic.valueLength() >= 3) {
            uint8_t r, g, b, color2R, color2G, color2B;
            r = data[0];
            g = data[1];
            b = data[2];
            Serial.print(F("[BLE] Display Effect Color 1: R="));
            Serial.print(r);
            Serial.print(F(" G="));
            Serial.print(g);
            Serial.print(F(" B="));
            Serial.println(b);
            // Get current color 2 to preserve it
            instance->controller.getDisplayGradientBottomColor(color2R, color2G, color2B);
            instance->controller.setDisplayGradientColors(r, g, b, color2R, color2G, color2B);

            // Also set the dual spiral and dual circle color (shared color for mode 4 & 5)
            instance->controller.setDisplayDualSpiralColor(r, g, b);
            instance->controller.setDisplayDualCircleColor(r, g, b);
        }
    }
}

void BLEManager::displayEffectColor2Written(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        if (characteristic.valueLength() >= 3) {
            uint8_t r, g, b, color1R, color1G, color1B;
            r = data[0];
            g = data[1];
            b = data[2];
            Serial.print(F("[BLE] Display Effect Color 2: R="));
            Serial.print(r);
            Serial.print(F(" G="));
            Serial.print(g);
            Serial.print(F(" B="));
            Serial.println(b);
            // Get current color 1 to preserve it
            instance->controller.getDisplayGradientTopColor(color1R, color1G, color1B);
            instance->controller.setDisplayGradientColors(color1R, color1G, color1B, r, g, b);

            // Also set the dual spiral and dual circle color to match color 1
            instance->controller.setDisplayDualSpiralColor(color1R, color1G, color1B);
            instance->controller.setDisplayDualCircleColor(color1R, color1G, color1B);
        }
    }
}

void BLEManager::displayEffectOption1Written(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        Serial.print(F("[BLE] Display Effect Option 1 (Thickness): "));
        Serial.println(*data);
        instance->controller.setDisplayEffectThickness(*data);
    }
}

void BLEManager::displayEffectOption2Written(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        Serial.print(F("[BLE] Display Effect Option 2 (Speed): "));
        Serial.println(*data);
        instance->controller.setDisplayEffectSpeed(*data);
    }
}

void BLEManager::displayEffectOption3Written(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        Serial.print(F("[BLE] Display Effect Option 3 (Direction Inverted): "));
        Serial.println(*data);
        instance->controller.setDisplayEffectDirectionInverted(*data);
    }
}

void BLEManager::rebootWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        // Any non-zero value triggers a reboot
        if (*data != 0) {
            Serial.println(F("Reboot requested via BLE"));
            instance->controller.reboot();
        }
    }
}
