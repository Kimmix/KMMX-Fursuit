#pragma once

// ESP32-S3-DevKitC-1 - Minimal Configuration (HUB75 Only)

// Random seed pin
#define RANDOM_PIN 1  // Use any unused ADC pin for true random

// HUB75 pins - Configured for ESP32-S3-DevKitC-1
// Using available GPIO pins that don't conflict with USB, PSRAM, or Flash
#define R1 4
#define G1 5
#define BL1 6
#define R2 7
#define G2 15
#define BL2 16
#define CH_A 18
#define CH_B 8
#define CH_C 3
#define CH_D 42
#define CH_E -1  // assign to any available pin if using two panels or 64x64 panels with 1/32 scan
#define CLK 41
#define LAT 40
#define OE 2

// Board capabilities (defined by platformio.ini, but can be checked here)
#ifndef HAS_ACCELEROMETER
#define HAS_ACCELEROMETER 0
#endif

#ifndef HAS_PROXIMITY
#define HAS_PROXIMITY 0
#endif

#ifndef HAS_HORN_LED
#define HAS_HORN_LED 0
#endif

#ifndef HAS_CHEEK_PANEL
#define HAS_CHEEK_PANEL 0
#endif

#ifndef HAS_I2C
#define HAS_I2C 0
#endif

// Dummy pin definitions for devices that are not present
// This prevents compilation errors when these pins are referenced
#define S3_SDA -1
#define S3_SCL -1
#define I2S_WS -1
#define I2S_SD -1
#define I2S_SCK -1
#define I2S_PORT I2S_NUM_0
#define LED_PWM_PIN -1
#define RGB_STATUS_PIN -1
#define ARGB_PIN -1

// Dummy configuration values for devices that are not present
// These are needed for conditional compilation but won't be used at runtime
const uint8_t argbCount = 0;
const uint8_t sideLEDBrightness = 0;
const uint16_t sideLEDAnimateInterval = 0;
const uint32_t sideColor1RGB = 0;
const uint32_t sideColor2RGB = 0;
const uint16_t sideLEDFadeInterval = 0;
const uint16_t sideLEDPositionChangeDelay = 0;

const uint8_t hornInitBrightness = 0;
const uint8_t hornPwmChannel = 0;
const uint16_t hornFrequency = 0;
const uint8_t hornResolution = 0;
const uint8_t hornMinBrightness = 0;
const uint8_t hornMaxBrightness = 0;
