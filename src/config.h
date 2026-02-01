#pragma once

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
#define USE_RLE_BITMAPS 1

// BLE settings
#define BLE_DEVICE_NAME "KMMX"
#define BLE_LOCAL_NAME "KMMX-BLE"
#define BLE_SERVICE_UUID "c1449275-bf34-40ab-979d-e34a1fdbb129"
#define BLE_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUID "9fdfd124-966b-44f7-8331-778c4d1512fc"
#define BLE_EYE_STATE_CHARACTERISTIC_UUID "49a36bb2-1c66-4e5c-8ff3-28e55a64beb3"
#define BLE_VISEME_CHARACTERISTIC_UUID "493d06f3-0fa0-4a90-88f1-ebaed0da9b80"

// HUB75 Config
const int panelResX = 64;
const int panelResY = 32;
const int panelsNumber = 2;
const int screenWidth = panelResX * panelsNumber;
const int screenHeight = panelResY;
const int panelInitBrightness = 255;
const int minRefreshRate = 240;
const bool doubleBuffer = true;

// SideLED configuration
#define ARGB_PIN 14     // Side ARGB Strip
const int argbCount = 24;                                                            // Number of LEDs
const short sideLEDBrightness = 255;                                                 // LED brightness
const long sideLEDAnimateInterval = 400;                                             // Animation interval in milliseconds
const short sideColor1Hue = 239, sideColor1Saturation = 255, sideColor1Value = 255;  // #FF446C (Reddish Pink)
const short sideColor2Hue = 22, sideColor2Saturation = 255, sideColor2Value = 255;   // #F9826C (Coral)

// HornLED PWM configuration
const int hornInitBrightness = 15;
const int hornPwmChannel = 0;
const int hornFrequency = 20000;
const int hornResolution = 8;
const int hornMinBrightness = 2;    // Minimum range for PWM
const int hornMaxBrightness = 200;   // Maximum range for PWM (255 can cause high heat!!)

// Controller etc. configuration
const int sensorUpdateInterval = 20;      // Sensor update interval in ms (50Hz for better responsiveness)
const short minimumPixelBrightness = 40;  // Minimum pixel brightness to draw on screen
const short noseWidth = 8;
const short noseHeight = 5;
const short noseOffsetX = 56;
const short noseOffsetY = 7;
const short eyeWidth = 32;
const short eyeHeight = 18;
const short eyeOffsetX = 7;
const short eyeOffsetY = 0;
const short mouthWidth = 50;
const short mouthHeight = 14;
const short mouthOffsetX = 14;
const short mouthOffsetY = 18;

// Boop configuration
const int IrMinThreshold = 100;         // Minimum proximity distance to activate
const int IrMaxThreshold = 900;         // Maximum range of proximity
const long BoopTimeMaxDuration = 2000;  // Total duration for calculate speed of boop

// Idle configuration
const unsigned long idleTimeout = 60000;           // Time till sleep in ms (60 seconds)
const unsigned long baselineUpdateDelay = 2000;    // Time to wait before updating baseline when still (2 seconds)
const float idleAccThreshold = 1.5;                // Magnitude change threshold to detect motion (m/s²)
const float sleepingAccThreshold = 0.7;            // Magnitude change threshold to wake up
const int motionHysteresisCount = 3;               // Number of motion detections needed to wake up
const int motionCounterDecayRate = 10;         // Samples between counter decay (10 = ~200ms at 50Hz)
const float gravityMagnitude = 9.81;               // Earth's gravity for reference (m/s²)
const bool enableIdleDebug = true;                 // Enable debug logging for idle detection

// Sample Rate and Samples
const double i2sSampleRate = 8000;
const int i2sSamples = 256;

// Viseme configuration
const int AHFreqMin = 700;                // Start freq of AH viseme
const int AHFreqMax = 1500;               // End freq of AH viseme
const int EEFreqMin = 1000;               // Start freq of EE viseme
const int EEFreqMax = 3000;               // End freq of EE viseme
const int OHFreqMin = 400;               // Start freq of OH viseme
const int OHFreqMax = 1100;               // End freq of OH viseme
const int OOFreqMin = 250;               // Start freq of OO viseme
const int OOFreqMax = 900;               // End freq of OO viseme
const int THFreqMin = 2800;               // Start freq of TH viseme
const int THFreqMax = 4000;               // End freq of TH viseme
const double visemeNoiseThreshold = 400;  // Minimum noise threshold for viseme to activate
const double visemeSmoothingAlpha = 0.2;  // Smoothing factor between 0 and 1
const double visemeDecayRate = 0.0003;    // Adjusted decay rate (units per millisecond)

// GooglyEye constants
const float G_SCALE = 1.0;      // Accel scale; no science, just looks good
const float ELASTICITY = 0.95;  // Edge-bounce coefficient (MUST be <1.0!)
const float DRAG = 0.997;       // Dampens motion slightly
const float EYE_RADIUS = 3.0;   // Radius of eye, floating-point pixel units
const float PUPIL_SIZE = 16.0;
const float PUPIL_RADIUS = PUPIL_SIZE / 2.0;           // Radius of pupil, same units
const float INNER_RADIUS = EYE_RADIUS - PUPIL_RADIUS;  // Radius of pupil motion
const short eyePupilWidth = 6;
const short eyePupilHeight = 6;
const short eyePupilOffsetX = 15;
const short eyePupilOffsetY = 5;