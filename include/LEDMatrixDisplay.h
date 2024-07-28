#pragma once

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "Bitmaps/accColor.h"
#include "config.h"

class LEDMatrixDisplay {
   private:
    MatrixPanel_I2S_DMA* matrix;
    const int panelWidth = PANEL_RES_X,
              panelHeight = PANEL_RES_Y;
    uint8_t panelBrightness = 40;
    int eyeWidth = 32, eyeHeight = 18,
        mouthWidth = 50, mouthHeight = 14;

    void getColorMap(const uint8_t lightness, const int row, uint8_t& r, uint8_t& g, uint8_t& b);
    void colorSpiral(const uint8_t brightness, const int startX, const int startY, const int row, const int col, uint8_t& r, uint8_t& g, uint8_t& b);
    void getBlackWhiteWave(const uint8_t brightness, const int row, const int col, uint8_t& r, uint8_t& g, uint8_t& b);
    void getColorWave(const uint8_t brightness, const int row, const int col, uint8_t& r, uint8_t& g, uint8_t& b);

   public:
    LEDMatrixDisplay();
    void render();
    void clearScreen();
    int getBrightnessValue();
    int getResX();
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void setTextColor(uint16_t color);
    void setCursor(int16_t x, int16_t y);
    void print(char str[]);
    void print(int n);
    void getTextBounds(const String& str, int16_t x, int16_t y, int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h);
    void setBrightnessValue(uint8_t value);

    // Drawing functions
    void drawFullscreen(const uint8_t* bitmap);
    void drawEye(const uint8_t* bitmap);
    void drawEye(const uint8_t* bitmapL, const uint8_t* bitmapR);
    void drawEyePupil(const uint8_t* bitmap, int x, int y);
    void drawNose(const uint8_t* bitmap);
    void drawMouth(const uint8_t* bitmap);
    void drawColorTest();
    void drawBitmap(const uint8_t* bitmap, int imageWidth, int imageHeight, int offsetX, int offsetY);
    void drawBitmap(const uint8_t* bitmapL, const uint8_t* bitmapR, int imageWidth, int imageHeight, int offsetX, int offsetY);
    void drawBitmap(const uint8_t* bitmap, int imageWidth, int imageHeight, int offsetX, int offsetY, uint8_t r, uint8_t g, uint8_t b);
};
