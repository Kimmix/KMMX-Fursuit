#pragma once

// Kimmix Controller V2 - Custom Board Pin Configuration

// I2C
#define S3_SDA 9
#define S3_SCL 3
#define RANDOM_PIN 2  // Use any unused ADC pin for true random

// I2S
#define I2S_WS 10
#define I2S_SD 12
#define I2S_SCK 11
#define I2S_PORT I2S_NUM_0

// HUB75 pins (Avoid QSPI pins)
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

// Other peripherals
#define LED_PWM_PIN 21  // Horn LED
#define RGB_STATUS_PIN 45  // RGB onboard pin
#define ARGB_PIN 14     // Side ARGB Strip

// Board capabilities (defined by platformio.ini, but can be checked here)
#ifndef HAS_ACCELEROMETER
#define HAS_ACCELEROMETER 1
#endif

#ifndef HAS_PROXIMITY
#define HAS_PROXIMITY 1
#endif

#ifndef HAS_HORN_LED
#define HAS_HORN_LED 1
#endif

#ifndef HAS_CHEEK_PANEL
#define HAS_CHEEK_PANEL 1
#endif

#ifndef HAS_I2C
#define HAS_I2C 1
#endif

// SideLED configuration
const uint8_t argbCount = 24;
const uint8_t sideLEDBrightness = 255;
const uint16_t sideLEDAnimateInterval = 400;
const uint32_t sideColor1RGB = 0xFF446C;  // #FF446C Reddish Pink
const uint32_t sideColor2RGB = 0xF9826C;  // #F9826C Coral
const uint16_t sideLEDFadeInterval = 500;
const uint16_t sideLEDPositionChangeDelay = 2000;

// HornLED PWM configuration
const uint8_t hornInitBrightness = 15;
const uint8_t hornPwmChannel = 0;
const uint16_t hornFrequency = 20000;
const uint8_t hornResolution = 8;
const uint8_t hornMinBrightness = 2;
const uint8_t hornMaxBrightness = 200;

// Note: Other common configurations (motion detection, viseme, etc.)
// are defined in the main config.h file
