// #include "Icons.h"
#include "main.h"

/**
 * @brief - return RGB of my custom defined gradient color
 * @param lightness 8-bit brightness of color
 * @param row current row of matrix output
 * @param uint8_t r - RGB888 color
 * @param uint8_t g - RGB888 color
 * @param uint8_t b - RGB888 color
 */
void myColor(const uint8_t lightness,
             int row,
             uint8_t& r,
             uint8_t& g,
             uint8_t& b) {
  r = (lightness * accColor[(row * 3)]) / 255;
  g = (lightness * accColor[(row * 3) + 1]) / 255;
  b = (lightness * accColor[(row * 3) + 2]) / 255;
}

/**
 * @brief - draw bitmap from grayscale
 * @param bitmap  byte array with 8-bit brightness bitmap
 */
void drawGSBitmap(const uint8_t bitmap[]) {
  int imageWidth = 64, imageHeight = 32;
  int offsetX = 0, offsetY = 0;
  int i, j, j2;
  for (i = 0; i < imageHeight; i++) {
    for (j = 0, j2 = 63; j < imageWidth; j++) {
      uint8_t r, g, b;
      myColor(bitmap[i * imageWidth + j], i + offsetY, r, g, b);
      matrix->drawPixelRGB888(offsetX + j, offsetY + i, r, g, b);
      matrix->drawPixelRGB888(-offsetX + PANEL_WIDTH + j2, offsetY + i, r, g,
                              b);
      j2--;
    }
  }
}

void drawEye(const uint8_t bitmap[]) {
  int imageWidth = 32, imageHeight = 18;
  int offsetX = 6, offsetY = 0;
  int i, j, j2;
  for (i = 0; i < imageHeight; i++) {
    for (j = 0, j2 = 63; j < imageWidth; j++) {
      uint8_t r, g, b;
      myColor(bitmap[i * imageWidth + j], i + offsetY, r, g, b);
      matrix->drawPixelRGB888(offsetX + j, offsetY + i, r, g, b);
      matrix->drawPixelRGB888(-offsetX + PANEL_WIDTH + j2, offsetY + i, r, g,
                              b);
      j2--;
    }
  }
}

void drawNose(const uint8_t bitmap[]) {
  int imageWidth = 10, imageHeight = 6;
  int offsetX = 54, offsetY = 6;
  int i, j, j2;
  for (i = 0; i < imageHeight; i++) {
    for (j = 0, j2 = 63; j < imageWidth; j++) {
      uint8_t r, g, b;
      myColor(bitmap[i * imageWidth + j], i + offsetY, r, g, b);
      matrix->drawPixelRGB888(offsetX + j, offsetY + i, r, g, b);
      matrix->drawPixelRGB888(-offsetX + PANEL_WIDTH + j2, offsetY + i, r, g,
                              b);
      j2--;
    }
  }
}

void drawMouth(const uint8_t bitmap[]) {
  int imageWidth = 50, imageHeight = 14;
  int offsetX = 14, offsetY = 18;
  int i, j, j2;
  for (i = 0; i < imageHeight; i++) {
    for (j = 0, j2 = 63; j < imageWidth; j++) {
      uint8_t r, g, b;
      myColor(bitmap[i * imageWidth + j], i + offsetY, r, g, b);
      matrix->drawPixelRGB888(offsetX + j, offsetY + i, r, g, b);
      matrix->drawPixelRGB888(-offsetX + PANEL_WIDTH + j2, offsetY + i, r, g,
                              b);
      j2--;
    }
  }
}