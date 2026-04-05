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
                                           displayGradientTopCharacteristic(BLE_DISPLAY_GRADIENT_TOP_CHARACTERISTIC_UUID, BLERead | BLEWrite, 3),
                                           displayGradientBottomCharacteristic(BLE_DISPLAY_GRADIENT_BOTTOM_CHARACTERISTIC_UUID, BLERead | BLEWrite, 3),
                                           displayDualSpiralThicknessCharacteristic(BLE_DISPLAY_DUAL_SPIRAL_THICKNESS_CHARACTERISTIC_UUID, BLERead | BLEWrite),
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
    protoService.addCharacteristic(displayGradientTopCharacteristic);
    protoService.addCharacteristic(displayGradientBottomCharacteristic);
    protoService.addCharacteristic(displayDualSpiralThicknessCharacteristic);
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

    // Set display color mode and gradient values
    displayColorModeCharacteristic.setValue(controller.getDisplayColorMode());

    uint8_t topR, topG, topB, bottomR, bottomG, bottomB;
    controller.getDisplayGradientTopColor(topR, topG, topB);
    controller.getDisplayGradientBottomColor(bottomR, bottomG, bottomB);
    uint8_t topColorData[3] = {topR, topG, topB};
    uint8_t bottomColorData[3] = {bottomR, bottomG, bottomB};
    displayGradientTopCharacteristic.setValue(topColorData, 3);
    displayGradientBottomCharacteristic.setValue(bottomColorData, 3);

    // Set dual spiral thickness value
    displayDualSpiralThicknessCharacteristic.setValue(controller.getDisplayDualSpiralThickness());

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
    displayGradientTopCharacteristic.setEventHandler(BLEWritten, displayGradientTopWritten);
    displayGradientBottomCharacteristic.setEventHandler(BLEWritten, displayGradientBottomWritten);
    displayDualSpiralThicknessCharacteristic.setEventHandler(BLEWritten, displayDualSpiralThicknessWritten);
    rebootCharacteristic.setEventHandler(BLEWritten, rebootWritten);

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

void BLEManager::displayGradientTopWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        if (characteristic.valueLength() >= 3) {
            uint8_t r, g, b, bottomR, bottomG, bottomB;
            r = data[0];
            g = data[1];
            b = data[2];
            Serial.print(F("[BLE] Display Gradient Top: R="));
            Serial.print(r);
            Serial.print(F(" G="));
            Serial.print(g);
            Serial.print(F(" B="));
            Serial.println(b);
            // Get current bottom color to preserve it
            instance->controller.getDisplayGradientBottomColor(bottomR, bottomG, bottomB);
            instance->controller.setDisplayGradientColors(r, g, b, bottomR, bottomG, bottomB);

            // Also set the dual spiral color to match the gradient top color (reuse for dual spiral)
            instance->controller.setDisplayDualSpiralColor(r, g, b);
        }
    }
}

void BLEManager::displayGradientBottomWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        if (characteristic.valueLength() >= 3) {
            uint8_t r, g, b, topR, topG, topB;
            r = data[0];
            g = data[1];
            b = data[2];
            Serial.print(F("[BLE] Display Gradient Bottom: R="));
            Serial.print(r);
            Serial.print(F(" G="));
            Serial.print(g);
            Serial.print(F(" B="));
            Serial.println(b);
            // Get current top color to preserve it
            instance->controller.getDisplayGradientTopColor(topR, topG, topB);
            instance->controller.setDisplayGradientColors(topR, topG, topB, r, g, b);

            // Also set the dual spiral color to match the gradient top color (reuse for dual spiral)
            instance->controller.setDisplayDualSpiralColor(topR, topG, topB);
        }
    }
}

void BLEManager::displayDualSpiralThicknessWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (instance) {
        const uint8_t* data = characteristic.value();
        Serial.print(F("[BLE] Dual Spiral Thickness: "));
        Serial.println(*data);
        instance->controller.setDisplayDualSpiralThickness(*data);
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
