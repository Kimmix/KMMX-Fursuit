#include "BLE.h"
#include <Arduino.h>
#include "config.h"
#include "BLE_UUIDs.h"

BLEManager* BLEManager::instance = nullptr;

// Constants for BLE advertising
static constexpr uint16_t BLE_ADV_MIN_INTERVAL = 0x20;      // 20ms (0x20 * 0.625ms)
static constexpr uint16_t BLE_ADV_MAX_INTERVAL = 0x40;      // 40ms (0x40 * 0.625ms)
static constexpr uint16_t BLE_APPEARANCE_DISPLAY = 0x03C0;  // Generic Display
static constexpr uint32_t BLE_RW = NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE;
static constexpr uint32_t BLE_WRITE = NIMBLE_PROPERTY::WRITE;

// Manufacturer data stored in flash memory (PROGMEM)
static const uint8_t BLE_MFG_DATA[] PROGMEM = {
    0xFF, 0xFF,          // Company ID (0xFFFF = custom/test)
    'K', 'M', 'M', 'X',  // KMMX identifier
    0x01, 0x00           // Version 1.0
};

// Simplified Server Callbacks - directly implement logic
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
        pServer->updateConnParams(connInfo.getConnHandle(), 12, 24, 0, 100);
        if (BLEManager::instance && BLEManager::instance->debugEnabled) {
            Serial.println(F("[BLE] Client connected"));
        }
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
        if (BLEManager::instance && BLEManager::instance->debugEnabled) {
            Serial.print(F("[BLE] Client disconnected, reason: "));
            Serial.println(reason);
        }
    }
};

static bool readByte(NimBLECharacteristic* characteristic, uint8_t& value) {
    const auto data = characteristic->getValue();
    if (data.length() != 1) return false;
    value = data[0];
    return true;
}

using ByteWriteHandler = void (*)(KMMXController&, uint8_t);
using ByteValidator = bool (*)(uint8_t);

static bool validAnyByte(uint8_t) { return true; }
static bool validBoolean(uint8_t value) { return value <= 1; }
static bool validNonZero(uint8_t value) { return value != 0; }

static void setDisplayBrightness(KMMXController& controller, uint8_t value) { controller.displayControl().setBrightnessValue(value); }
static void setEyeState(KMMXController& controller, uint8_t value) { controller.setEye(value); }
static void setMouthState(KMMXController& controller, uint8_t value) { controller.setMouth(value); }
static void setVisemeState(KMMXController& controller, uint8_t value) { controller.setViseme(value); }
static void setHornBrightness(KMMXController& controller, uint8_t value) { controller.setHornBrightness(value); }
static void setCheekBrightness(KMMXController& controller, uint8_t value) { controller.setCheekBrightness(value); }
static void setDisplayColorMode(KMMXController& controller, uint8_t value) { controller.displayControl().setColorMode(value); }
static void setDisplayEffectThickness(KMMXController& controller, uint8_t value) { controller.displayControl().setEffectThickness(value); }
static void setDisplayEffectSpeed(KMMXController& controller, uint8_t value) { controller.displayControl().setEffectSpeed(value); }
static void setDisplayEffectDirection(KMMXController& controller, uint8_t value) { controller.displayControl().setEffectDirectionInverted(value); }
static void rebootController(KMMXController& controller, uint8_t) { controller.reboot(); }
static void setSlotMachineEnabled(KMMXController& controller, uint8_t value) { controller.setSlotMachineEnabled(value); }
static void triggerGlitch(KMMXController& controller, uint8_t value) { controller.triggerGlitch(value); }
static void setMotionFlags(KMMXController& controller, uint8_t value) { controller.setMotionEnableFlags(value); }
static void setTapSensitivity(KMMXController& controller, uint8_t value) { controller.setTapSensitivity(value); }
static void setGlitchIntensity(KMMXController& controller, uint8_t value) { controller.setGlitchIntensity(value); }

#if HAS_FAN_CONTROL
static void setFanSpeed(KMMXController& controller, uint8_t value) { controller.setFanSpeed(value); }
#endif

class ByteWriteCallbacks : public NimBLECharacteristicCallbacks {
   public:
    ByteWriteCallbacks(const __FlashStringHelper* label, ByteWriteHandler handler, ByteValidator validator = validAnyByte, bool hex = false)
        : label(label), handler(handler), validator(validator), hex(hex) {}

    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
        if (!BLEManager::instance) return;
        uint8_t value;
        if (!readByte(pCharacteristic, value)) return;
        if (!validator(value)) return;
        if (BLEManager::instance->debugEnabled) {
            Serial.print(label);
            if (hex) Serial.println(value, HEX);
            else Serial.println(value);
        }
        handler(BLEManager::instance->controller, value);
    }

   private:
    const __FlashStringHelper* label;
    ByteWriteHandler handler;
    ByteValidator validator;
    bool hex;
};

class CheekBgColorCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
        if (!BLEManager::instance) return;
        if (pCharacteristic->getValue().length() >= 3) {
            const uint8_t* data = pCharacteristic->getValue().data();
            if (BLEManager::instance->debugEnabled) {
                Serial.print(F("[BLE] Cheek BG Color: R="));
                Serial.print(data[0]);
                Serial.print(F(" G="));
                Serial.print(data[1]);
                Serial.print(F(" B="));
                Serial.println(data[2]);
            }
            BLEManager::instance->controller.setCheekBackgroundColor(data[0], data[1], data[2]);
        }
    }
};

class CheekFadeColorCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
        if (!BLEManager::instance) return;
        if (pCharacteristic->getValue().length() >= 3) {
            const uint8_t* data = pCharacteristic->getValue().data();
            if (BLEManager::instance->debugEnabled) {
                Serial.print(F("[BLE] Cheek Fade Color: R="));
                Serial.print(data[0]);
                Serial.print(F(" G="));
                Serial.print(data[1]);
                Serial.print(F(" B="));
                Serial.println(data[2]);
            }
            BLEManager::instance->controller.setCheekFadeColor(data[0], data[1], data[2]);
        }
    }
};

class DisplayEffectColor1Callbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
        if (!BLEManager::instance) return;
        if (pCharacteristic->getValue().length() >= 3) {
            const uint8_t* data = pCharacteristic->getValue().data();
            uint8_t r = data[0], g = data[1], b = data[2];
            if (BLEManager::instance->debugEnabled) {
                Serial.print(F("[BLE] Display Effect Color 1: R="));
                Serial.print(r);
                Serial.print(F(" G="));
                Serial.print(g);
                Serial.print(F(" B="));
                Serial.println(b);
            }

            // Get current color 2 to preserve it
            uint8_t color2R, color2G, color2B;
            auto& display = BLEManager::instance->controller.displayControl();
            display.getGradientBottomColor(color2R, color2G, color2B);
            display.setGradientColors(r, g, b, color2R, color2G, color2B);

            // Also set the dual spiral and dual circle color
            display.setDualSpiralColor(r, g, b);
            display.setDualCircleColor(r, g, b);
        }
    }
};

class DisplayEffectColor2Callbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
        if (!BLEManager::instance) return;
        if (pCharacteristic->getValue().length() >= 3) {
            const uint8_t* data = pCharacteristic->getValue().data();
            uint8_t r = data[0], g = data[1], b = data[2];
            if (BLEManager::instance->debugEnabled) {
                Serial.print(F("[BLE] Display Effect Color 2: R="));
                Serial.print(r);
                Serial.print(F(" G="));
                Serial.print(g);
                Serial.print(F(" B="));
                Serial.println(b);
            }

            // Get current color 1 to preserve it
            uint8_t color1R, color1G, color1B;
            auto& display = BLEManager::instance->controller.displayControl();
            display.getGradientTopColor(color1R, color1G, color1B);
            display.setGradientColors(color1R, color1G, color1B, r, g, b);

            // Also set the dual spiral and dual circle color to match color 1
            display.setDualSpiralColor(color1R, color1G, color1B);
            display.setDualCircleColor(color1R, color1G, color1B);
        }
    }
};

enum class VisemeParameter {
    EnvelopeAttack,
    EnvelopeRelease,
    NoiseGateMultiplier,
    NoiseFloorMin,
    AhScale,
    EeScale,
    OhScale,
    OoScale,
    ThScale,
    LoudnessExponent,
    LoudnessSmoothing,
    LoudnessMax,
    LoudnessMidBoost,
};

class VisemeParameterCallbacks : public NimBLECharacteristicCallbacks {
   public:
    explicit VisemeParameterCallbacks(VisemeParameter parameter) : parameter(parameter) {}

    void onWrite(NimBLECharacteristic* characteristic, NimBLEConnInfo&) override {
        if (!BLEManager::instance || characteristic->getValue().length() != sizeof(float)) return;

        float value;
        memcpy(&value, characteristic->getValue().data(), sizeof(value));
        if (!isfinite(value)) return;

        auto& controller = BLEManager::instance->controller;
        auto& viseme = controller.viseme();
        switch (parameter) {
            case VisemeParameter::EnvelopeAttack:
                if (value < 0.1f || value > 0.9f) return;
                viseme.setEnvelopeAttack(value);
                break;
            case VisemeParameter::EnvelopeRelease:
                if (value < 0.01f || value > 0.5f) return;
                viseme.setEnvelopeRelease(value);
                break;
            case VisemeParameter::NoiseGateMultiplier:
                if (value < 1.0f || value > 3.0f) return;
                viseme.setNoiseGateMultiplier(value);
                break;
            case VisemeParameter::NoiseFloorMin:
                if (value < 1.0f || value > visemeNoiseFloorCap) return;
                viseme.setNoiseFloorMin(value);
                break;
            case VisemeParameter::AhScale:
            case VisemeParameter::EeScale:
            case VisemeParameter::OhScale:
            case VisemeParameter::OoScale:
            case VisemeParameter::ThScale:
                if (value < 0.1f || value > 5.0f) return;
                if (parameter == VisemeParameter::AhScale) viseme.setAhScale(value);
                else if (parameter == VisemeParameter::EeScale) viseme.setEeScale(value);
                else if (parameter == VisemeParameter::OhScale) viseme.setOhScale(value);
                else if (parameter == VisemeParameter::OoScale) viseme.setOoScale(value);
                else viseme.setThScale(value);
                break;
            case VisemeParameter::LoudnessExponent:
                if (value < 0.2f || value > 2.0f) return;
                viseme.setLoudnessExponent(value);
                break;
            case VisemeParameter::LoudnessSmoothing:
                if (value < 0.05f || value > 1.0f) return;
                viseme.setLoudnessSmoothing(value);
                break;
            case VisemeParameter::LoudnessMax:
                if (value < 1.0f || value > 20.0f) return;
                viseme.setLoudnessMax(value);
                break;
            case VisemeParameter::LoudnessMidBoost:
                if (value < 0.5f || value > 2.0f) return;
                viseme.setLoudnessMidBoost(value);
                break;
        }
    }

   private:
    VisemeParameter parameter;
};

BLEManager& BLEManager::getInstance(KMMXController& ctrl) {
    if (!instance) {
        instance = new BLEManager(ctrl);
    }
    return *instance;
}

BLEManager::BLEManager(KMMXController& ctrl) : controller(ctrl),
                                               pServer(nullptr),
                                               pService(nullptr)
{
}

void BLEManager::setup() {
    Serial.println(F("Booting BLE..."));

    // Initialize NimBLE with device name
    NimBLEDevice::init(BLE_DEVICE_NAME);

    // Set power level for better range
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);  // +9dBm

    // Create BLE Server
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
    pServer->advertiseOnDisconnect(true);  // Automatically restart advertising on disconnect

    // Create BLE Service
    pService = pServer->createService(BLE_SERVICE_UUID);

    // Create characteristics with Read & Write properties
    auto* displayBrightnessCharacteristic = pService->createCharacteristic(
        BLE_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* eyeStateCharacteristic = pService->createCharacteristic(
        BLE_EYE_STATE_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* mouthStateCharacteristic = pService->createCharacteristic(
        BLE_MOUTH_STATE_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* visemeCharacteristic = pService->createCharacteristic(
        BLE_VISEME_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* hornBrightnessCharacteristic = pService->createCharacteristic(
        BLE_HORN_BRIGHTNESS_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* cheekBrightnessCharacteristic = pService->createCharacteristic(
        BLE_CHEEK_BRIGHTNESS_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* cheekBgColorCharacteristic = pService->createCharacteristic(
        BLE_CHEEK_BG_COLOR_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* cheekFadeColorCharacteristic = pService->createCharacteristic(
        BLE_CHEEK_FADE_COLOR_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* displayColorModeCharacteristic = pService->createCharacteristic(
        BLE_DISPLAY_COLOR_MODE_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* displayEffectColor1Characteristic = pService->createCharacteristic(
        BLE_DISPLAY_EFFECT_COLOR1_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* displayEffectColor2Characteristic = pService->createCharacteristic(
        BLE_DISPLAY_EFFECT_COLOR2_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* displayEffectOption1Characteristic = pService->createCharacteristic(
        BLE_DISPLAY_EFFECT_OPTION1_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* displayEffectOption2Characteristic = pService->createCharacteristic(
        BLE_DISPLAY_EFFECT_OPTION2_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* displayEffectOption3Characteristic = pService->createCharacteristic(
        BLE_DISPLAY_EFFECT_OPTION3_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* rebootCharacteristic = pService->createCharacteristic(
        BLE_REBOOT_CHARACTERISTIC_UUID,
        BLE_WRITE);

    auto* slotMachineEnableCharacteristic = pService->createCharacteristic(
        BLE_SLOT_MACHINE_ENABLE_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* glitchTriggerCharacteristic = pService->createCharacteristic(
        BLE_GLITCH_TRIGGER_CHARACTERISTIC_UUID,
        BLE_WRITE);

    auto* motionEnableFlagsCharacteristic = pService->createCharacteristic(
        BLE_MOTION_ENABLE_FLAGS_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* tapSensitivityCharacteristic = pService->createCharacteristic(
        BLE_TAP_SENSITIVITY_CHARACTERISTIC_UUID,
        BLE_RW);

    auto* glitchIntensityCharacteristic = pService->createCharacteristic(
        BLE_GLITCH_INTENSITY_CHARACTERISTIC_UUID,
        BLE_RW);

    // Fan Control Characteristics
    #if HAS_FAN_CONTROL
    auto* fanSpeedCharacteristic = pService->createCharacteristic(
        BLE_FAN_SPEED_CHARACTERISTIC_UUID,
        BLE_RW);
    #endif

    // Viseme Advanced Parameter Characteristics
    auto* visemeEnvelopeAttackCharacteristic = pService->createCharacteristic(
        BLE_VISEME_ENVELOPE_ATTACK_UUID,
        BLE_RW);

    auto* visemeEnvelopeReleaseCharacteristic = pService->createCharacteristic(
        BLE_VISEME_ENVELOPE_RELEASE_UUID,
        BLE_RW);

    auto* visemeNoiseGateMultiplierCharacteristic = pService->createCharacteristic(
        BLE_VISEME_NOISE_GATE_MULTIPLIER_UUID,
        BLE_RW);

    auto* visemeNoiseFloorMinCharacteristic = pService->createCharacteristic(
        BLE_VISEME_NOISE_FLOOR_MIN_UUID,
        BLE_RW);

    auto* visemeAhScaleCharacteristic = pService->createCharacteristic(
        BLE_VISEME_AH_SCALE_UUID,
        BLE_RW);

    auto* visemeEeScaleCharacteristic = pService->createCharacteristic(
        BLE_VISEME_EE_SCALE_UUID,
        BLE_RW);

    auto* visemeOhScaleCharacteristic = pService->createCharacteristic(
        BLE_VISEME_OH_SCALE_UUID,
        BLE_RW);

    auto* visemeOoScaleCharacteristic = pService->createCharacteristic(
        BLE_VISEME_OO_SCALE_UUID,
        BLE_RW);

    auto* visemeThScaleCharacteristic = pService->createCharacteristic(
        BLE_VISEME_TH_SCALE_UUID,
        BLE_RW);

    auto* visemeLoudnessExponentCharacteristic = pService->createCharacteristic(
        BLE_VISEME_LOUDNESS_EXPONENT_UUID,
        BLE_RW);

    auto* visemeLoudnessSmoothingCharacteristic = pService->createCharacteristic(
        BLE_VISEME_LOUDNESS_SMOOTHING_UUID,
        BLE_RW);

    auto* visemeLoudnessMaxCharacteristic = pService->createCharacteristic(
        BLE_VISEME_LOUDNESS_MAX_UUID,
        BLE_RW);

    auto* visemeLoudnessMidBoostCharacteristic = pService->createCharacteristic(
        BLE_VISEME_LOUDNESS_MID_BOOST_UUID,
        BLE_RW);

    // Set default values for each characteristic
    uint8_t brightnessValue = controller.displayControl().getBrightnessValue();
    displayBrightnessCharacteristic->setValue(&brightnessValue, 1);

    uint8_t eyeValue = 0x00;
    eyeStateCharacteristic->setValue(&eyeValue, 1);

    uint8_t mouthValue = 0x00;
    mouthStateCharacteristic->setValue(&mouthValue, 1);

    uint8_t visemeValue = controller.getViseme();
    visemeCharacteristic->setValue(&visemeValue, 1);

    uint8_t slotMachineValue = controller.getSlotMachineEnabled();
    slotMachineEnableCharacteristic->setValue(&slotMachineValue, 1);

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
    uint8_t colorMode = controller.displayControl().getColorMode();
    displayColorModeCharacteristic->setValue(&colorMode, 1);

    // Set effect color values
    uint8_t color1R, color1G, color1B, color2R, color2G, color2B;
    controller.displayControl().getGradientTopColor(color1R, color1G, color1B);
    controller.displayControl().getGradientBottomColor(color2R, color2G, color2B);
    uint8_t color1Data[3] = {color1R, color1G, color1B};
    uint8_t color2Data[3] = {color2R, color2G, color2B};
    displayEffectColor1Characteristic->setValue(color1Data, 3);
    displayEffectColor2Characteristic->setValue(color2Data, 3);

    // Set effect option values
    uint8_t thickness = controller.displayControl().getEffectThickness();
    displayEffectOption1Characteristic->setValue(&thickness, 1);

    uint8_t speed = controller.displayControl().getEffectSpeed();
    displayEffectOption2Characteristic->setValue(&speed, 1);

    uint8_t direction = controller.displayControl().getEffectDirectionInverted();
    displayEffectOption3Characteristic->setValue(&direction, 1);

    // Set motion detection & glitch control default values
    uint8_t motionFlags = controller.getMotionEnableFlags();
    motionEnableFlagsCharacteristic->setValue(&motionFlags, 1);

    uint8_t tapSens = controller.getTapSensitivity();
    tapSensitivityCharacteristic->setValue(&tapSens, 1);

    uint8_t glitchInt = controller.getGlitchIntensity();
    glitchIntensityCharacteristic->setValue(&glitchInt, 1);

    // Set fan control default values
    #if HAS_FAN_CONTROL
    uint8_t fanSpeed = controller.getFanSpeed();
    fanSpeedCharacteristic->setValue(&fanSpeed, 1);
    #endif

    // Set viseme advanced parameter default values
    auto& viseme = controller.viseme();
    float envAttack = viseme.getEnvelopeAttack();
    visemeEnvelopeAttackCharacteristic->setValue(reinterpret_cast<uint8_t*>(&envAttack), sizeof(float));

    float envRelease = viseme.getEnvelopeRelease();
    visemeEnvelopeReleaseCharacteristic->setValue(reinterpret_cast<uint8_t*>(&envRelease), sizeof(float));

    float noiseGateMultiplier = viseme.getNoiseGateMultiplier();
    visemeNoiseGateMultiplierCharacteristic->setValue(reinterpret_cast<uint8_t*>(&noiseGateMultiplier), sizeof(float));

    float noiseMin = viseme.getNoiseFloorMin();
    visemeNoiseFloorMinCharacteristic->setValue(reinterpret_cast<uint8_t*>(&noiseMin), sizeof(float));

    float ahScale = viseme.getAhScale();
    visemeAhScaleCharacteristic->setValue(reinterpret_cast<uint8_t*>(&ahScale), sizeof(float));

    float eeScale = viseme.getEeScale();
    visemeEeScaleCharacteristic->setValue(reinterpret_cast<uint8_t*>(&eeScale), sizeof(float));

    float ohScale = viseme.getOhScale();
    visemeOhScaleCharacteristic->setValue(reinterpret_cast<uint8_t*>(&ohScale), sizeof(float));

    float ooScale = viseme.getOoScale();
    visemeOoScaleCharacteristic->setValue(reinterpret_cast<uint8_t*>(&ooScale), sizeof(float));

    float thScale = viseme.getThScale();
    visemeThScaleCharacteristic->setValue(reinterpret_cast<uint8_t*>(&thScale), sizeof(float));

    float loudnessExponent = viseme.getLoudnessExponent();
    visemeLoudnessExponentCharacteristic->setValue(reinterpret_cast<uint8_t*>(&loudnessExponent), sizeof(float));

    float loudnessSmoothing = viseme.getLoudnessSmoothing();
    visemeLoudnessSmoothingCharacteristic->setValue(reinterpret_cast<uint8_t*>(&loudnessSmoothing), sizeof(float));

    float loudnessMax = viseme.getLoudnessMax();
    visemeLoudnessMaxCharacteristic->setValue(reinterpret_cast<uint8_t*>(&loudnessMax), sizeof(float));

    float loudnessMidBoost = viseme.getLoudnessMidBoost();
    visemeLoudnessMidBoostCharacteristic->setValue(reinterpret_cast<uint8_t*>(&loudnessMidBoost), sizeof(float));

    // Set callbacks for each characteristic (simple, direct callbacks)
    displayBrightnessCharacteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Display Brightness: "), setDisplayBrightness));
    eyeStateCharacteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Eye State: "), setEyeState));
    mouthStateCharacteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Mouth State: "), setMouthState));
    visemeCharacteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Viseme: "), setVisemeState, validBoolean));
    hornBrightnessCharacteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Horn Brightness: "), setHornBrightness));
    cheekBrightnessCharacteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Cheek Brightness: "), setCheekBrightness));
    cheekBgColorCharacteristic->setCallbacks(new CheekBgColorCallbacks());
    cheekFadeColorCharacteristic->setCallbacks(new CheekFadeColorCallbacks());
    displayColorModeCharacteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Display Color Mode: "), setDisplayColorMode));
    displayEffectColor1Characteristic->setCallbacks(new DisplayEffectColor1Callbacks());
    displayEffectColor2Characteristic->setCallbacks(new DisplayEffectColor2Callbacks());
    displayEffectOption1Characteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Display Effect Option 1 (Thickness): "), setDisplayEffectThickness));
    displayEffectOption2Characteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Display Effect Option 2 (Speed): "), setDisplayEffectSpeed));
    displayEffectOption3Characteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Display Effect Option 3 (Direction Inverted): "), setDisplayEffectDirection));
    rebootCharacteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Reboot requested: "), rebootController, validNonZero));
    slotMachineEnableCharacteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Slot Machine: "), setSlotMachineEnabled, validBoolean));
    glitchTriggerCharacteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Glitch Trigger: "), triggerGlitch));
    motionEnableFlagsCharacteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Motion Enable Flags: 0x"), setMotionFlags, validAnyByte, true));
    tapSensitivityCharacteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Tap Sensitivity: "), setTapSensitivity));
    glitchIntensityCharacteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Glitch Intensity: "), setGlitchIntensity));

    // Set fan control callbacks
    #if HAS_FAN_CONTROL
    fanSpeedCharacteristic->setCallbacks(new ByteWriteCallbacks(F("[BLE] Fan Speed: "), setFanSpeed));
    #endif

    // Set viseme advanced parameter callbacks
    visemeEnvelopeAttackCharacteristic->setCallbacks(new VisemeParameterCallbacks(VisemeParameter::EnvelopeAttack));
    visemeEnvelopeReleaseCharacteristic->setCallbacks(new VisemeParameterCallbacks(VisemeParameter::EnvelopeRelease));
    visemeNoiseGateMultiplierCharacteristic->setCallbacks(new VisemeParameterCallbacks(VisemeParameter::NoiseGateMultiplier));
    visemeNoiseFloorMinCharacteristic->setCallbacks(new VisemeParameterCallbacks(VisemeParameter::NoiseFloorMin));
    visemeAhScaleCharacteristic->setCallbacks(new VisemeParameterCallbacks(VisemeParameter::AhScale));
    visemeEeScaleCharacteristic->setCallbacks(new VisemeParameterCallbacks(VisemeParameter::EeScale));
    visemeOhScaleCharacteristic->setCallbacks(new VisemeParameterCallbacks(VisemeParameter::OhScale));
    visemeOoScaleCharacteristic->setCallbacks(new VisemeParameterCallbacks(VisemeParameter::OoScale));
    visemeThScaleCharacteristic->setCallbacks(new VisemeParameterCallbacks(VisemeParameter::ThScale));
    visemeLoudnessExponentCharacteristic->setCallbacks(new VisemeParameterCallbacks(VisemeParameter::LoudnessExponent));
    visemeLoudnessSmoothingCharacteristic->setCallbacks(new VisemeParameterCallbacks(VisemeParameter::LoudnessSmoothing));
    visemeLoudnessMaxCharacteristic->setCallbacks(new VisemeParameterCallbacks(VisemeParameter::LoudnessMax));
    visemeLoudnessMidBoostCharacteristic->setCallbacks(new VisemeParameterCallbacks(VisemeParameter::LoudnessMidBoost));

    // Start the server (this automatically starts all services)
    pServer->start();

    // Configure advertising
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();

    // Create advertising data with manufacturer info and appearance
    NimBLEAdvertisementData advData;
    advData.setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);  // General discoverable, BR/EDR not supported
    advData.setCompleteServices(BLEUUID(BLE_SERVICE_UUID));
    advData.setAppearance(BLE_APPEARANCE_DISPLAY);

    // Add manufacturer data from flash memory
    uint8_t mfgDataBuffer[sizeof(BLE_MFG_DATA)];
    memcpy_P(mfgDataBuffer, BLE_MFG_DATA, sizeof(BLE_MFG_DATA));
    advData.setManufacturerData(std::string((char*)mfgDataBuffer, sizeof(mfgDataBuffer)));

    // Create scan response data with device name
    NimBLEAdvertisementData scanResponseData;
    scanResponseData.setName(BLE_DEVICE_NAME);

    pAdvertising->setAdvertisementData(advData);
    pAdvertising->setScanResponseData(scanResponseData);
    pAdvertising->setMinInterval(BLE_ADV_MIN_INTERVAL);
    pAdvertising->setMaxInterval(BLE_ADV_MAX_INTERVAL);

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

void BLEManager::update() {
}
