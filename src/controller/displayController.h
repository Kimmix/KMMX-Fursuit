#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "FastLED_Pixel_Buffer.h"

// ---- P3 LED Matrix Panel ----
// HUB75E pinout
// R1 | G1
// B1 | GND
// R2 | G2
// B2 | E
//  A | B
//  C | D
// CLK| LAT
// OE | GND

// Default library pin configuration for the reference you can redefine only ones you need later on object creation
// #define R1 25
// #define G1 26
// #define BL1 27
// #define R2 14
// #define G2 12
// #define BL2 13
// #define CH_A 23
// #define CH_B 19
// #define CH_C 5
// #define CH_D 17
// #define CH_E -1 // assign to any available pin if using panels with 1/32 scan
// #define CLK 16
// #define LAT 4

// extern MatrixPanel_I2S_DMA* matrix;
// extern VirtualMatrixPanel_FastLED_Pixel_Buffer* FastLED_Pixel_Buff;

#define PANEL_WIDTH 64
#define PANEL_HEIGHT 32
#define PANELS_NUMBER 2
#define SCREEN_WIDTH PANEL_WIDTH* PANELS_NUMBER
#define SCREEN_HEIGHT PANEL_HEIGHT

// *** Led Matrix ***
MatrixPanel_I2S_DMA* matrix = nullptr;
VirtualMatrixPanel_FastLED_Pixel_Buffer* FastLED_Pixel_Buff = nullptr;

class DisplayController {
public:
  void init() {
    // ------ Setup P3 LED Matrix Pannel ------
    HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANELS_NUMBER);
    // mxconfig.clkphase = false;
    matrix = new MatrixPanel_I2S_DMA(mxconfig);
    // matrix->setBrightness8(96);  // 0-255
    matrix->clearScreen();
    delay(500);
    if (not matrix->begin())
      Serial.println("****** I2S memory allocation failed ***********");
    FastLED_Pixel_Buff = new VirtualMatrixPanel_FastLED_Pixel_Buffer((*matrix), 1, PANELS_NUMBER, PANEL_WIDTH, PANEL_HEIGHT, true, false);
    if (not FastLED_Pixel_Buff->allocateMemory())
      Serial.println("****** Unable to find enough memory for the FastLED pixel buffer! ***********");
  }

};