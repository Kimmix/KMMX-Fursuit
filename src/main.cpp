#include "main.h"
#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "Icons.h"

// HUB75E pinout
// R1 | G1
// B1 | GND
// R2 | G2
// B2 | E
//  A | B
//  C | D
// CLK| LAT
// OE | GND

/*  Default library pin configuration for the reference
  you can redefine only ones you need later on object creation
#define R1 25
#define G1 26
#define BL1 27
#define R2 14
#define G2 12
#define BL2 13
#define CH_A 23
#define CH_B 19
#define CH_C 5
#define CH_D 17
#define CH_E -1 // assign to any available pin if using panels with 1/32 scan
#define CLK 16
#define LAT 4
#define OE 15
*/

MatrixPanel_I2S_DMA* matrix = nullptr;

void buffclear(CRGB* buf) {
  memset(buf, 0x00, NUM_LEDS * sizeof(CRGB));  // flush buffer to black
}

void drawColorTest() {
  for (int i = 0; i < 64; i++) {
    uint8_t r, g, b;
    myColor(255, i, r, g, b);
    matrix->drawPixelRGB888(0, i, r, g, b);
  }
}

void setup() {
  Serial.begin(BAUD_RATE);
  // ------ Setup P3 LED Matrix Pannel ------
  HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANELS_NUMBER);
  mxconfig.driver = HUB75_I2S_CFG::ICN2038S;
  matrix = new MatrixPanel_I2S_DMA(mxconfig);
  matrix->begin();
  matrix->setBrightness8(20);
  // matrix->clearScreen();
  // drawGSBitmap(face_gs);
  drawEye(eyeDefault);
  drawNose(noseDefault);
  drawMouth(mouthDefault);
  drawColorTest();
}

void loop() {
  // Serial.println("Fill screen: RED");
  // matrix->fillScreenRGB888(255, 0, 0);
  // delay(PATTERN_DELAY);
  // draw_eye(0, 0, myEye);
  // delay(PATTERN_DELAY);
}