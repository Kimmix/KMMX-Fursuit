#pragma once

#include <stdint.h>

// ESP32S3 Custom board PINS
// I2C
#define S3_SDA 9
#define S3_SCL 3
#define RANDOM_PIN 2  // Use any usused ADC pin for true random
// I2S
#define I2S_WS 10
#define I2S_SD 12
#define I2S_SCK 11
// I2S Processor
#define I2S_PORT I2S_NUM_0
// HUB75 pin (Avoid and QSPI pins)
#define R1 4
#define G1 5
#define BL1 6
#define R2 7
#define G2 15
#define BL2 16
#define CH_A 18
#define CH_B 8
#define CH_C 19
#define CH_D 20
#define CH_E 17  // assign to any available pin if using two panels or 64x64 panels with 1/32 scan
#define CLK 41
#define LAT 40
#define OE 39
// Other
#define LED_PWM_PIN 21  // Horn LED
#define RGB_STATUS_PIN 45  // RGB onboard pin

// Enable RLE bitmap support
#define USE_RLE_BITMAPS 0

// BLE settings - can be overridden by platformio.ini build flags
#ifndef BLE_DEVICE_NAME
#define BLE_DEVICE_NAME "KMMX"
#endif

#ifndef BLE_LOCAL_NAME
#define BLE_LOCAL_NAME "KMMX-BLE"
#endif

#define BLE_SERVICE_UUID "c1449275-bf34-40ab-979d-e34a1fdbb129"
#define BLE_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUID "9fdfd124-966b-44f7-8331-778c4d1512fc"
#define BLE_EYE_STATE_CHARACTERISTIC_UUID "49a36bb2-1c66-4e5c-8ff3-28e55a64beb3"
#define BLE_MOUTH_STATE_CHARACTERISTIC_UUID "f6a7b8c9-d0e1-4f5a-b1c2-3d4e5f6a7b8c"
#define BLE_VISEME_CHARACTERISTIC_UUID "493d06f3-0fa0-4a90-88f1-ebaed0da9b80"
#define BLE_HORN_BRIGHTNESS_CHARACTERISTIC_UUID "a1b2c3d4-e5f6-4a5b-8c9d-0e1f2a3b4c5d"
#define BLE_CHEEK_BRIGHTNESS_CHARACTERISTIC_UUID "b2c3d4e5-f6a7-4b5c-9d0e-1f2a3b4c5d6e"
#define BLE_CHEEK_BG_COLOR_CHARACTERISTIC_UUID "c3d4e5f6-a7b8-4c5d-9e0f-1a2b3c4d5e6f"
#define BLE_CHEEK_FADE_COLOR_CHARACTERISTIC_UUID "d4e5f6a7-b8c9-4d5e-9f0a-1b2c3d4e5f6a"
#define BLE_DISPLAY_COLOR_MODE_CHARACTERISTIC_UUID "f5a6b7c8-d9e0-4f5a-b0c1-2d3e4f5a6b7c"
#define BLE_DISPLAY_EFFECT_COLOR1_CHARACTERISTIC_UUID "a6b7c8d9-e0f1-4a5b-c1d2-3e4f5a6b7c8d"
#define BLE_DISPLAY_EFFECT_COLOR2_CHARACTERISTIC_UUID "b7c8d9e0-f1a2-4b5c-d2e3-4f5a6b7c8d9e"
#define BLE_DISPLAY_EFFECT_OPTION1_CHARACTERISTIC_UUID "c7d8e9f0-a1b2-4c5d-e2f3-4a5b6c7d8e9f"
#define BLE_DISPLAY_EFFECT_OPTION2_CHARACTERISTIC_UUID "e7f8a9b0-c1d2-4e5f-a2b3-4c5d6e7f8a9b"
#define BLE_DISPLAY_EFFECT_OPTION3_CHARACTERISTIC_UUID "f7a8b9c0-d1e2-4f5a-b2c3-4d5e6f7a8b9c"
#define BLE_REBOOT_CHARACTERISTIC_UUID "e5f6a7b8-c9d0-4e5f-a0b1-2c3d4e5f6a7b"

// HUB75 Config
const uint8_t panelResX = 64;
const uint8_t panelResY = 32;
const uint8_t panelsNumber = 2;
const uint8_t screenWidth = panelResX * panelsNumber;
const uint8_t screenHeight = panelResY;
const uint8_t panelInitBrightness = 255;
const uint16_t minRefreshRate = 240;
const bool doubleBuffer = true;

// SideLED configuration
#define ARGB_PIN 14     // Side ARGB Strip
const uint8_t argbCount = 24;                                                   // Number of LEDs
const uint8_t sideLEDBrightness = 255;                                          // LED brightness
const uint16_t sideLEDAnimateInterval = 400;                                    // Animation interval in milliseconds
const uint32_t sideColor1RGB = 0xFF446C;                                        // #FF446C Reddish Pink (RGB hex)
const uint32_t sideColor2RGB = 0xF9826C;                                        // #F9826C Coral (RGB hex)
const uint16_t sideLEDFadeInterval = 500;                                       // Fade interval in milliseconds
const uint16_t sideLEDPositionChangeDelay = 2000;                               // Position change delay in milliseconds

// HornLED PWM configuration
const uint8_t hornInitBrightness = 15;
const uint8_t hornPwmChannel = 0;
const uint16_t hornFrequency = 20000;
const uint8_t hornResolution = 8;
const uint8_t hornMinBrightness = 2;    // Minimum range for PWM
const uint8_t hornMaxBrightness = 200;   // Maximum range for PWM (255 can cause high heat!!)

// Controller etc. configuration
const uint8_t sensorUpdateInterval = 20;       // Sensor update interval in ms (50Hz for better responsiveness)
const uint8_t minimumPixelBrightness = 40;     // Minimum pixel brightness to draw on screen
const uint8_t noseWidth = 8;
const uint8_t noseHeight = 5;
const uint8_t noseOffsetX = 56;
const uint8_t noseOffsetY = 7;
const uint8_t eyeWidth = 32;
const uint8_t eyeHeight = 18;
const uint8_t eyeOffsetX = 15;
const uint8_t eyeOffsetY = 0;
const uint8_t mouthWidth = 50;
const uint8_t mouthHeight = 14;
const uint8_t mouthOffsetX = 14;
const uint8_t mouthOffsetY = 18;

// Boop configuration
const uint16_t boopMinThreshold = 100;         // Minimum proximity distance to activate
const uint16_t boopMaxThreshold = 900;         // Maximum range of proximity
const uint16_t boopMaxDuration = 2000;         // Total duration for calculate speed of boop

// Idle configuration
const uint32_t idleTimeout = 300000;            // Time till sleep in ms (300 seconds)
const uint16_t baselineUpdateDelay = 2000;     // Time to wait before updating baseline when still (2 seconds)
const float idleAccThreshold = 1.5f;           // Magnitude change threshold to detect motion (m/s²)
const float sleepingAccThreshold = 0.7f;       // Magnitude change threshold to wake up
const uint8_t motionHysteresisCount = 3;       // Number of motion detections needed to wake up
const uint8_t motionCounterDecayRate = 10;     // Samples between counter decay (10 = ~200ms at 50Hz)
const float gravityMagnitude = 9.81f;          // Earth's gravity for reference (m/s²)
const bool enableIdleDebug = false;            // Enable debug logging for idle detection

// Motion Detection Configuration
// Global debug flag for all motion detection features
const bool enableMotionDebug = true;            // Enable detailed debug logging for all motion detection (shows continuous amplitude/intensity values)
const uint16_t motionDetectionStartupDelay = 5000; // Delay before motion detection starts (ms) - increased to account for BLE init - prevents false triggers during sensor initialization

// Tilt Detection - Detects sustained head tilt for curious/confused expressions
const bool enableTiltDetection = false;
const float tiltThreshold = 4.0f;               // m/s² threshold for tilt detection (reduced for easier triggering)
const float tiltNeutralThreshold = 3.0f;        // m/s² threshold to return to neutral (increased for easier return)
const uint16_t tiltSustainTime = 500;           // Time to hold tilt before triggering (ms)
const uint16_t tiltDebounceTime = 300;          // Cooldown between tilt changes (ms)
const uint16_t tiltDirectionChangeCooldown = 3000;  // Cooldown when switching from left/right to forward/back (ms) - prevents rapid direction changes

// Upside Down Detection - Detects when the character is held upside down
const bool enableUpsideDownDetection = false;
const float upsideDownThreshold = -7.0f;            // m/s² threshold for Y-axis (negative = upside down)
const uint16_t upsideDownSustainTime = 1500;        // Time to hold upside down before triggering (ms)
const uint16_t upsideDownDebounceTime = 500;        // Cooldown between state changes (ms)

// Petting Detection - Detects quick pats/taps (spike-based) for contentment
const bool enablePettingDetection = true;
const bool enablePettingCooldownAfterTilt = true;   // Enable cooldown after tilt to prevent immediate petting
const uint16_t pettingCooldownAfterTilt = 1000;     // Cooldown duration after tilt ends (ms)

const float pettingSpikeThreshold = 0.8f;           // m/s² threshold for detecting a gentle pat/tap spike
const uint16_t pettingSpikeCooldown = 300;          // Minimum time between individual spikes (ms) - prevents double-counting

// Dynamic happiness system parameters
const float pettingHappinessPerPat = 30.0f;         // Happiness added per pat (0-100 scale)
const float pettingHappinessTrigger = 80.0f;        // Happiness level to trigger SMILE response (0-100)
const float pettingHappinessDecayRate = 15.0f;      // Happiness decay per second when not petting
const float pettingHappinessEndThreshold = 20.0f;   // Happiness level below which response ends (allows natural fade-out)
const float pettingDeltaTimeMax = 1.0f;             // Maximum delta time in seconds to accept (sanity check for time jumps)

// Sample Rate and Samples
const float i2sSampleRate = 8000.0f;
const uint16_t i2sSamples = 256;

// Viseme configuration
const uint16_t visemeAhFreqMin = 700;          // Start freq of AH viseme
const uint16_t visemeAhFreqMax = 1500;         // End freq of AH viseme
const uint16_t visemeEeFreqMin = 1000;         // Start freq of EE viseme
const uint16_t visemeEeFreqMax = 3000;         // End freq of EE viseme
const uint16_t visemeOhFreqMin = 400;          // Start freq of OH viseme
const uint16_t visemeOhFreqMax = 1100;         // End freq of OH viseme
const uint16_t visemeOoFreqMin = 250;          // Start freq of OO viseme
const uint16_t visemeOoFreqMax = 900;          // End freq of OO viseme
const uint16_t visemeThFreqMin = 2800;         // Start freq of TH viseme
const uint16_t visemeThFreqMax = 4000;         // End freq of TH viseme
const float visemeNoiseThreshold = 400.0f;     // Minimum noise threshold for viseme to activate
const float visemeSmoothingAlpha = 0.2f;       // Smoothing factor between 0 and 1
const float visemeDecayRate = 0.0003f;         // Adjusted decay rate (units per millisecond)

// GooglyEye configuration
const float googlyAccelScale = 1.0f;           // Accel scale; no science, just looks good
const float googlyElasticity = 0.95f;          // Edge-bounce coefficient (MUST be <1.0!)
const float googlyDrag = 0.997f;               // Dampens motion slightly
const float googlyEyeRadius = 3.0f;            // Radius of eye, floating-point pixel units
const float googlyPupilSize = 16.0f;
const float googlyPupilRadius = googlyPupilSize / 2.0f;             // Radius of pupil, same units
const float googlyInnerRadius = googlyEyeRadius - googlyPupilRadius;  // Radius of pupil motion
const uint8_t googlyPupilWidth = 6;
const uint8_t googlyPupilHeight = 6;
const uint8_t googlyPupilOffsetX = eyeOffsetX + 8;
const uint8_t googlyPupilOffsetY = 5;