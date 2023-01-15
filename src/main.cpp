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

#define PANEL_WIDTH 64
#define PANEL_HEIGHT 32
#define PANELS_NUMBER 2

#define PANE_WIDTH PANEL_WIDTH* PANELS_NUMBER
#define PANE_HEIGHT PANEL_HEIGHT
#define NUM_LEDS PANE_WIDTH* PANE_HEIGHT

MatrixPanel_I2S_DMA* matrix = nullptr;

#define PATTERN_DELAY 2000

void buffclear(CRGB* buf) {
  memset(buf, 0x00, NUM_LEDS * sizeof(CRGB));  // flush buffer to black
}

void draw_888_face_con(
    int x,
    int y,
    const uint8_t ani[])  // this draws the face, x and y is an offset
{
  // Draw screen1
  int imageHeight = 32;
  int imageWidth = 64;
  int counter = 0;
  for (int yy = 0; yy < imageHeight; yy++) {
    for (int xx = 0; xx < imageWidth; xx++) {
      matrix->drawPixelRGB888(xx + x, yy + y, ani[counter], ani[counter + 1],
                              ani[counter + 2]);

      counter = counter + 3;
    }
  }
  // Draw screen2
  imageHeight = 32;
  imageWidth = 64;
  counter = 0;
  for (int yy = 0; yy < imageHeight; yy++) {
    for (int xx = 63; xx > -1; xx--) {
      matrix->drawPixelRGB888(xx + 64, yy + y, ani[counter], ani[counter + 1],
                              ani[counter + 2]);

      counter = counter + 3;
    }
  }
}

void drawEye(int bitmap[]) {
  int imageWidth = 25;
  int imageHeight = 14;
  int offsetX = 10;
  int offsetY = 2;
  int i, j, j2;
  for (i = 0; i < imageHeight; i++) {
    for (j = 0, j2 = 63; j < imageWidth; j++) {
      matrix->drawPixel(offsetX + j, offsetY + i,
                        (uint16_t)bitmap[i * imageWidth + j]);
      matrix->drawPixel(-offsetX + PANEL_WIDTH + j2, offsetY + i,
                        (uint16_t)bitmap[i * imageWidth + j]);
      j2--;
    }
  }
}

void setup() {
  Serial.begin(BAUD_RATE);
  // Setup P3 LED Matrix Pannel
  HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANELS_NUMBER);
  mxconfig.driver = HUB75_I2S_CFG::ICN2038S;
  matrix = new MatrixPanel_I2S_DMA(mxconfig);
  matrix->begin();
  matrix->setBrightness8(20);
  // matrix->clearScreen();
  // draw_888_face_con(0, 0, myFace);
  drawEye(eyeDefault);
}

void loop() {
  // Serial.println("Fill screen: RED");
  // matrix->fillScreenRGB888(255, 0, 0);
  // delay(PATTERN_DELAY);
  // draw_eye(0, 0, myEye);
  // delay(PATTERN_DELAY);
}