#include "BLE.h"
#include <Arduino.h>
#include "config.h"

BLEManager* BLEManager::instance = nullptr;

// Define static callback instances (initialized with function pointers)
ServerCallbacks BLEManager::serverCallbacks;
CharacteristicCallbacks BLEManager::displayBrightnessCallbacks(BLEManager::onDisplayBrightnessWrite);
CharacteristicCallbacks BLEManager::eyeStateCallbacks(BLEManager::onEyeStateWrite);
CharacteristicCallbacks BLEManager::mouthStateCallbacks(BLEManager::onMouthStateWrite);
CharacteristicCallbacks BLEManager::visemeCallbacks(BLEManager::onVisemeStateWrite);
CharacteristicCallbacks BLEManager::hornBrightnessCallbacks(BLEManager::onHornBrightnessWrite);
CharacteristicCallbacks BLEManager::cheekBrightnessCallbacks(BLEManager::onCheekBrightnessWrite);
CharacteristicCallbacks BLEManager::cheekBgColorCallbacks(BLEManager::onCheekBgColorWrite);
CharacteristicCallbacks BLEManager::cheekFadeColorCallbacks(BLEManager::onCheekFadeColorWrite);
CharacteristicCallbacks BLEManager::displayColorModeCallbacks(BLEManager::onDisplayColorModeWrite);
CharacteristicCallbacks BLEManager::displayEffectColor1Callbacks(BLEManager::onDisplayEffectColor1Write);
CharacteristicCallbacks BLEManager::displayEffectColor2Callbacks(BLEManager::onDisplayEffectColor2Write);
CharacteristicCallbacks BLEManager::displayEffectOption1Callbacks(BLEManager::onDisplayEffectOption1Write);
CharacteristicCallbacks BLEManager::displayEffectOption2Callbacks(BLEManager::onDisplayEffectOption2Write);
CharacteristicCallbacks BLEManager::displayEffectOption3Callbacks(BLEManager::onDisplayEffectOption3Write);
CharacteristicCallbacks BLEManager::rebootCallbacks(BLEManager::onRebootWrite);

// ServerCallbacks implementations
void ServerCallbacks::onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
    Serial.println(F("[ServerCallbacks] onConnect triggered"));
    BLEManager::onConnect();
}

void ServerCallbacks::onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
    Serial.print(F("[ServerCallbacks] onDisconnect triggered, reason: "));
    Serial.println(reason);
    BLEManager::onDisconnect();
}

BLEManager& BLEManager::getInstance(KMMXController& ctrl) {
    if (!instance) {
        instance = new BLEManager(ctrl);
    }
    return *instance;
}

BLEManager::BLEManager(KMMXController& ctrl) : controller(ctrl),
                                               pServer(nullptr),
                                               pService(nullptr),
                                               displayBrightnessCharacteristic(nullptr),
                                               eyeStateCharacteristic(nullptr),
                                               mouthStateCharacteristic(nullptr),
                                               visemeCharacteristic(nullptr),
                                               hornBrightnessCharacteristic(nullptr),
                                               cheekBrightnessCharacteristic(nullptr),
                                               cheekBgColorCharacteristic(nullptr),
                                               cheekFadeColorCharacteristic(nullptr),
                                               displayColorModeCharacteristic(nullptr),
                                               displayEffectColor1Characteristic(nullptr),
                                               displayEffectColor2Characteristic(nullptr),
                                               displayEffectOption1Characteristic(nullptr),
                                               displayEffectOption2Characteristic(nullptr),
                                               displayEffectOption3Characteristic(nullptr),
                                               rebootCharacteristic(nullptr) {
}

void BLEManager::setup() {
    Serial.println(F("Booting BLE..."));
    pinMode(LED_BUILTIN, OUTPUT);

    // Initialize NimBLE with device name
    NimBLEDevice::init(BLE_DEVICE_NAME);

    // Set power level for better range (optional)
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);  // +9dBm

    // Create BLE Server
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(&serverCallbacks);
    pServer->advertiseOnDisconnect(true);  // Automatically restart advertising on disconnect

    // Create BLE Service
    pService = pServer->createService(BLE_SERVICE_UUID);

    // Create characteristics with Read & Write properties
    displayBrightnessCharacteristic = pService->createCharacteristic(
        BLE_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    eyeStateCharacteristic = pService->createCharacteristic(
        BLE_EYE_STATE_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    mouthStateCharacteristic = pService->createCharacteristic(
        BLE_MOUTH_STATE_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    visemeCharacteristic = pService->createCharacteristic(
        BLE_VISEME_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    hornBrightnessCharacteristic = pService->createCharacteristic(
        BLE_HORN_BRIGHTNESS_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    cheekBrightnessCharacteristic = pService->createCharacteristic(
        BLE_CHEEK_BRIGHTNESS_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    cheekBgColorCharacteristic = pService->createCharacteristic(
        BLE_CHEEK_BG_COLOR_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    cheekFadeColorCharacteristic = pService->createCharacteristic(
        BLE_CHEEK_FADE_COLOR_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    displayColorModeCharacteristic = pService->createCharacteristic(
        BLE_DISPLAY_COLOR_MODE_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    displayEffectColor1Characteristic = pService->createCharacteristic(
        BLE_DISPLAY_EFFECT_COLOR1_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    displayEffectColor2Characteristic = pService->createCharacteristic(
        BLE_DISPLAY_EFFECT_COLOR2_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    displayEffectOption1Characteristic = pService->createCharacteristic(
        BLE_DISPLAY_EFFECT_OPTION1_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    displayEffectOption2Characteristic = pService->createCharacteristic(
        BLE_DISPLAY_EFFECT_OPTION2_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    displayEffectOption3Characteristic = pService->createCharacteristic(
        BLE_DISPLAY_EFFECT_OPTION3_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    rebootCharacteristic = pService->createCharacteristic(
        BLE_REBOOT_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::WRITE);

    // Set default values for each characteristic
    uint8_t brightnessValue = controller.getDisplayBrightness();
    displayBrightnessCharacteristic->setValue(&brightnessValue, 1);

    uint8_t eyeValue = 0x00;
    eyeStateCharacteristic->setValue(&eyeValue, 1);

    uint8_t mouthValue = 0x00;
    mouthStateCharacteristic->setValue(&mouthValue, 1);

    uint8_t visemeValue = controller.getViseme();
    visemeCharacteristic->setValue(&visemeValue, 1);

    uint8_t hornValue = controller.getHornBrightness();
    hornBrightnessCharacteristic->setValue(&hornValue, 1);

    uint8_t cheekValue = controller.getCheekBrightness();
    cheekBrightnessCharacteristic->setValue(&cheekValue, 1);

    // Set default color values (RGB format)
    uint32_t bgColor = controller.getCheekBackgroundColor();
    uint8_t bgColorData[3] = {(uint8_t)(bgColor >> 16), (uint8_t)(bgColor >> 8), (uint8_t)bgColor};
    cheekBgColorCharacteristic->setValue(bgColorData, 3);

    uint32_t fadeColor = controller.getCheekFadeColor();
    uint8_t fadeColorData[3] = {(uint8_t)(fadeColor >> 16), (uint8_t)(fadeColor >> 8), (uint8_t)fadeColor};
    cheekFadeColorCharacteristic->setValue(fadeColorData, 3);

    // Set display color mode
    uint8_t colorMode = controller.getDisplayColorMode();
    displayColorModeCharacteristic->setValue(&colorMode, 1);

    // Set effect color values
    uint8_t color1R, color1G, color1B, color2R, color2G, color2B;
    controller.getDisplayGradientTopColor(color1R, color1G, color1B);
    controller.getDisplayGradientBottomColor(color2R, color2G, color2B);
    uint8_t color1Data[3] = {color1R, color1G, color1B};
    uint8_t color2Data[3] = {color2R, color2G, color2B};
    displayEffectColor1Characteristic->setValue(color1Data, 3);
    displayEffectColor2Characteristic->setValue(color2Data, 3);

    // Set effect option values
    uint8_t thickness = controller.getDisplayEffectThickness();
    displayEffectOption1Characteristic->setValue(&thickness, 1);

    uint8_t speed = controller.getDisplayEffectSpeed();
    displayEffectOption2Characteristic->setValue(&speed, 1);

    uint8_t direction = controller.getDisplayEffectDirectionInverted();
    displayEffectOption3Characteristic->setValue(&direction, 1);

    // Set callbacks for each characteristic (using static instances - no memory leaks!)
    displayBrightnessCharacteristic->setCallbacks(&displayBrightnessCallbacks);
    eyeStateCharacteristic->setCallbacks(&eyeStateCallbacks);
    mouthStateCharacteristic->setCallbacks(&mouthStateCallbacks);
    visemeCharacteristic->setCallbacks(&visemeCallbacks);
    hornBrightnessCharacteristic->setCallbacks(&hornBrightnessCallbacks);
    cheekBrightnessCharacteristic->setCallbacks(&cheekBrightnessCallbacks);
    cheekBgColorCharacteristic->setCallbacks(&cheekBgColorCallbacks);
    cheekFadeColorCharacteristic->setCallbacks(&cheekFadeColorCallbacks);
    displayColorModeCharacteristic->setCallbacks(&displayColorModeCallbacks);
    displayEffectColor1Characteristic->setCallbacks(&displayEffectColor1Callbacks);
    displayEffectColor2Characteristic->setCallbacks(&displayEffectColor2Callbacks);
    displayEffectOption1Characteristic->setCallbacks(&displayEffectOption1Callbacks);
    displayEffectOption2Characteristic->setCallbacks(&displayEffectOption2Callbacks);
    displayEffectOption3Characteristic->setCallbacks(&displayEffectOption3Callbacks);
    rebootCharacteristic->setCallbacks(&rebootCallbacks);

    // Start the server (this automatically starts all services)
    pServer->start();

    // Configure advertising
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();

    // Create advertising data with manufacturer info and appearance
    NimBLEAdvertisementData advData;
    advData.setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);  // General discoverable, BR/EDR not supported
    advData.setCompleteServices(BLEUUID(BLE_SERVICE_UUID));  // Service UUID
    advData.setAppearance(0x03C0);  // Generic Display appearance

    // Add manufacturer data (0xFFFF = Custom/Test Company ID)
    // Format: [Company ID Low, Company ID High, ...custom data...]
    uint8_t mfgData[] = {
        0xFF, 0xFF,           // Company ID (0xFFFF = custom)
        'K', 'M', 'M', 'X',   // KMMX identifier
        0x01, 0x00            // Version 1.0
    };
    advData.setManufacturerData(std::string((char*)mfgData, sizeof(mfgData)));

    // Create scan response data with device name
    NimBLEAdvertisementData scanResponseData;
    scanResponseData.setName(BLE_DEVICE_NAME);

    pAdvertising->setAdvertisementData(advData);
    pAdvertising->setScanResponseData(scanResponseData);
    pAdvertising->setMinInterval(0x20);  // 20ms intervals (0x20 * 0.625ms = 20ms)
    pAdvertising->setMaxInterval(0x40);  // 40ms intervals (0x40 * 0.625ms = 40ms)

    // Start advertising
    if (pAdvertising->start()) {
        Serial.println(F("Bluetooth® device active, waiting for connections..."));
    } else {
        Serial.println(F("Failed to start advertising!"));
    }
}

bool BLEManager::isConnected() const {
    return pServer && pServer->getConnectedCount() > 0;
}

void BLEManager::onConnect() {
    Serial.println(F("[BLEManager] Connected event"));
    digitalWrite(LED_BUILTIN, HIGH);
}

void BLEManager::onDisconnect() {
    Serial.println(F("[BLEManager] Disconnected event"));
    digitalWrite(LED_BUILTIN, LOW);
    // Note: Advertising automatically restarts thanks to advertiseOnDisconnect(true)
}

void BLEManager::onDisplayBrightnessWrite(const uint8_t* data, size_t length) {
    if (instance && length >= 1) {
        Serial.print(F("[BLE] Display Brightness: "));
        Serial.println(*data);
        instance->controller.setDisplayBrightness(static_cast<int>(*data));
    }
}

void BLEManager::onEyeStateWrite(const uint8_t* data, size_t length) {
    if (instance && length >= 1) {
        Serial.print(F("[BLE] Eye State: "));
        Serial.println(*data);
        instance->controller.setEye(static_cast<int>(*data));
    }
}

void BLEManager::onMouthStateWrite(const uint8_t* data, size_t length) {
    if (instance && length >= 1) {
        Serial.print(F("[BLE] Mouth State: "));
        Serial.println(*data);
        instance->controller.setMouth(static_cast<int>(*data));
    }
}

void BLEManager::onVisemeStateWrite(const uint8_t* data, size_t length) {
    if (instance && length >= 1) {
        Serial.print(F("[BLE] Viseme: "));
        Serial.println(*data);
        instance->controller.setViseme(static_cast<int>(*data));
    }
}

void BLEManager::onHornBrightnessWrite(const uint8_t* data, size_t length) {
    if (instance && length >= 1) {
        Serial.print(F("[BLE] Horn Brightness: "));
        Serial.println(*data);
        instance->controller.setHornBrightness(static_cast<int>(*data));
    }
}

void BLEManager::onCheekBrightnessWrite(const uint8_t* data, size_t length) {
    if (instance && length >= 1) {
        Serial.print(F("[BLE] Cheek Brightness: "));
        Serial.println(*data);
        instance->controller.setCheekBrightness(static_cast<int>(*data));
    }
}

void BLEManager::onCheekBgColorWrite(const uint8_t* data, size_t length) {
    if (instance && length >= 3) {
        Serial.print(F("[BLE] Cheek BG Color: R="));
        Serial.print(data[0]);
        Serial.print(F(" G="));
        Serial.print(data[1]);
        Serial.print(F(" B="));
        Serial.println(data[2]);
        instance->controller.setCheekBackgroundColor(data[0], data[1], data[2]);
    }
}

void BLEManager::onCheekFadeColorWrite(const uint8_t* data, size_t length) {
    if (instance && length >= 3) {
        Serial.print(F("[BLE] Cheek Fade Color: R="));
        Serial.print(data[0]);
        Serial.print(F(" G="));
        Serial.print(data[1]);
        Serial.print(F(" B="));
        Serial.println(data[2]);
        instance->controller.setCheekFadeColor(data[0], data[1], data[2]);
    }
}

void BLEManager::onDisplayColorModeWrite(const uint8_t* data, size_t length) {
    if (instance && length >= 1) {
        Serial.print(F("[BLE] Display Color Mode: "));
        Serial.println(*data);
        instance->controller.setDisplayColorMode(static_cast<uint8_t>(*data));
    }
}

void BLEManager::onDisplayEffectColor1Write(const uint8_t* data, size_t length) {
    if (instance && length >= 3) {
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

void BLEManager::onDisplayEffectColor2Write(const uint8_t* data, size_t length) {
    if (instance && length >= 3) {
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

void BLEManager::onDisplayEffectOption1Write(const uint8_t* data, size_t length) {
    if (instance && length >= 1) {
        Serial.print(F("[BLE] Display Effect Option 1 (Thickness): "));
        Serial.println(*data);
        instance->controller.setDisplayEffectThickness(*data);
    }
}

void BLEManager::onDisplayEffectOption2Write(const uint8_t* data, size_t length) {
    if (instance && length >= 1) {
        Serial.print(F("[BLE] Display Effect Option 2 (Speed): "));
        Serial.println(*data);
        instance->controller.setDisplayEffectSpeed(*data);
    }
}

void BLEManager::onDisplayEffectOption3Write(const uint8_t* data, size_t length) {
    if (instance && length >= 1) {
        Serial.print(F("[BLE] Display Effect Option 3 (Direction Inverted): "));
        Serial.println(*data);
        instance->controller.setDisplayEffectDirectionInverted(*data);
    }
}

void BLEManager::onRebootWrite(const uint8_t* data, size_t length) {
    if (instance && length >= 1) {
        // Any non-zero value triggers a reboot
        if (*data != 0) {
            Serial.println(F("Reboot requested via BLE"));
            instance->controller.reboot();
        }
    }
}
