#pragma once
#include <FastLED.h>

// Undefine existing definitions
#ifdef SDA
#undef SDA
#endif
#ifdef SCL
#undef SCL
#endif
#ifdef LED_BUILTIN
#undef LED_BUILTIN
#endif

// ESP32S3 Custom board PINS
#define LED_BUILTIN 45
#define RANDOM_PIN 2  // Use any usused ADC pin for true random
// I2C
#define SDA 9
#define SCL 3
// I2S
#define I2S_WS 10
#define I2S_SD 12
#define I2S_SCK 11
// I2S Processor
#define I2S_PORT I2S_NUM_0
// HUB75 Config
#define PANEL_RES_X 64
#define PANEL_RES_Y 32
#define PANELS_NUMBER 2
#define SCREEN_WIDTH PANEL_RES_X* PANELS_NUMBER
#define SCREEN_HEIGHT PANEL_RES_Y
// HUB75 pin
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
#define ARGB 14
#define LED_PWM 21
#define IR_PIN GPIO_NUM_35

// BLE settings
#define BLE_DEVICE_NAME "KMMX"
#define BLE_LOCAL_NAME "KMMX-BLE"
#define BLE_SERVICE_UUID "c1449275-bf34-40ab-979d-e34a1fdbb129"
#define BLE_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUID "9fdfd124-966b-44f7-8331-778c4d1512fc"
#define BLE_EYE_STATE_CHARACTERISTIC_UUID "49a36bb2-1c66-4e5c-8ff3-28e55a64beb3"
#define BLE_VISEME_CHARACTERISTIC_UUID "493d06f3-0fa0-4a90-88f1-ebaed0da9b80"

// SideLED configuration
#define LED_TYPE WS2812                            // WS2812 or WS2812B, depending on your LEDs
#define COLOR_ORDER GRB                            // GRB or RGB, depending on your LEDs
const int argbNum = 24;                            // Number of LEDs
const short sideLEDBrightness = 255;               // LED brightness
const unsigned long sideLEDAnimateInterval = 400;  // Animation interval in milliseconds
const CHSV sideColor1HSV(239, 255, 255);           // Start color: #FF446C (Reddish Pink)
const CHSV sideColor2HSV(22, 255, 255);            // End color: #F9826C (Coral)

// HornLED configuration
const unsigned short hornInitBrightness = 30;
const unsigned short hornPin = 2;  // Define your LED_PWM pin number here
const unsigned short hornPwmChannel = 0;
const unsigned short hornFrequency = 20000;
const unsigned short hornResolution = 8;
const unsigned short hornMinBrightness = 50;
const unsigned short hornMaxBrightness = 200;

// Controller etc. configuration
const bool vsync = false;
const int sensorUpdateInterval = 100;     // Sensor update interval in milliseconds
const int sleepThreshold = 5000;          // Time in milliseconds before the controller goes to sleep
const float accelerationThreshold = 0.6;  // Threshold for detecting stillness in accelerometer readings

// Display configuration
const short frametime = 7;  // Frame time in milliseconds (~144hz)

// Viseme configuration
const int AH_MIN = 600;
const int AH_MAX = 1200;
const int EE_MIN = 1000;
const int EE_MAX = 2000;
const int OH_MIN = 1800;
const int OH_MAX = 2800;
const int OO_MIN = 2600;
const int OO_MAX = 3600;
const int TH_MIN = 3400;
const int TH_MAX = 4000;
const int VISEME_FRAME = 20;
const double NOISE_THRESHOLD = 400;
const double SMOOTHING_ALPHA = 0.2;  // smoothing factor between 0 and 1

// Sample Rate and Samples
const double SAMPLE_RATE = 8000;
const int SAMPLES = 256;

// GooglyEye constants
const float G_SCALE = 1.0;      // Accel scale; no science, just looks good
const float ELASTICITY = 0.95;  // Edge-bounce coefficient (MUST be <1.0!)
const float DRAG = 0.997;       // Dampens motion slightly
const float EYE_RADIUS = 3.0;   // Radius of eye, floating-point pixel units
const float PUPIL_SIZE = 16.0;
const float PUPIL_RADIUS = PUPIL_SIZE / 2.0;           // Radius of pupil, same units
const float INNER_RADIUS = EYE_RADIUS - PUPIL_RADIUS;  // Radius of pupil motion