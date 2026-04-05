#pragma once

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "config.h"
#include "Bitmaps/Bitmaps.h"
#include "ColorEffects.h"

class Hub75DMA {
   private:
    MatrixPanel_I2S_DMA* matrix;  // Pointer to the matrix panel object
    const uint8_t panelWidth = panelResX;   // Panel width from config
    const uint8_t panelHeight = panelResY;  // Panel height from config
    uint8_t panelBrightness = panelInitBrightness; // Default brightness

    // Color effects engine
    ColorEffects colorEffects = ColorEffects(panelResX, panelResY);
    uint8_t colorMode = 0;  // 0 = default gradient, 1 = spiral/vortex, etc.

    // Helper functions to generate colors and patterns
    /**
     * @brief Generates a color based on the lightness, row, and column.
     * @param lightness Lightness value (0-255)
     * @param row Row index
     * @param col Column index
     * @param r Reference to red component (output)
     * @param g Reference to green component (output)
     * @param b Reference to blue component (output)
     */
    void getColorMap(const uint8_t lightness, const int row, const int col, uint8_t& r, uint8_t& g, uint8_t& b);

    /**
     * @brief Generates a spiral color pattern.
     * @param brightness Brightness value (0-255)
     * @param startX Starting X coordinate
     * @param startY Starting Y coordinate
     * @param row Row index
     * @param col Column index
     * @param r Reference to red component (output)
     * @param g Reference to green component (output)
     * @param b Reference to blue component (output)
     */
    void colorSpiral(const uint8_t brightness, const int startX, const int startY, const int row, const int col, uint8_t& r, uint8_t& g, uint8_t& b);

    /**
     * @brief Generates a black and white wave pattern.
     * @param brightness Brightness value (0-255)
     * @param row Row index
     * @param col Column index
     * @param r Reference to red component (output)
     * @param g Reference to green component (output)
     * @param b Reference to blue component (output)
     */
    void getBlackWhiteWave(const uint8_t brightness, const int row, const int col, uint8_t& r, uint8_t& g, uint8_t& b);

    /**
     * @brief Generates a color wave pattern.
     * @param brightness Brightness value (0-255)
     * @param row Row index
     * @param col Column index
     * @param r Reference to red component (output)
     * @param g Reference to green component (output)
     * @param b Reference to blue component (output)
     */
    void getColorWave(const uint8_t brightness, const int row, const int col, uint8_t& r, uint8_t& g, uint8_t& b);

   public:
    /**
     * @brief Constructor for the Hub75DMA class.
     */
    Hub75DMA();

    int getRefreshRate();

    /**
     * @brief Renders the buffer to the screen.
     */
    void flipDMABuffer();

    /**
     * @brief Clears the screen.
     */
    void clearScreen();

    /**
     * @brief Gets the current brightness value.
     * @return Current brightness value.
     */
    int getBrightnessValue();

    /**
     * @brief Gets the resolution width.
     * @return Resolution width.
     */
    int getResX();

    /**
     * @brief Converts RGB values to 565 format.
     * @param r Red component (0-255)
     * @param g Green component (0-255)
     * @param b Blue component (0-255)
     * @return Color in 565 format.
     */
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Draws a rectangle.
     * @param x X coordinate of the top-left corner
     * @param y Y coordinate of the top-left corner
     * @param w Width of the rectangle
     * @param h Height of the rectangle
     * @param color Color of the rectangle in 565 format
     */
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    /**
     * @brief Draws a pixel.
     * @param x X coordinate
     * @param y Y coordinate
     * @param color Color of the pixel in 565 format
     */
    void drawPixel(int16_t x, int16_t y, uint16_t color);

    /**
     * @brief Sets the text color.
     * @param color Color of the text in 565 format
     */
    void setTextColor(uint16_t color);

    /**
     * @brief Sets the cursor position.
     * @param x X coordinate
     * @param y Y coordinate
     */
    void setCursor(int16_t x, int16_t y);

    /**
     * @brief Prints a string.
     * @param str String to print
     */
    void print(char str[]);

    /**
     * @brief Prints an integer.
     * @param n Integer to print
     */
    void print(int n);

    /**
     * @brief Gets the bounds of the text.
     * @param str String to measure
     * @param x X coordinate
     * @param y Y coordinate
     * @param x1 Reference to X coordinate of the text bounds (output)
     * @param y1 Reference to Y coordinate of the text bounds (output)
     * @param w Reference to width of the text bounds (output)
     * @param h Reference to height of the text bounds (output)
     */
    void getTextBounds(const String& str, int16_t x, int16_t y, int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h);

    /**
     * @brief Sets the brightness value.
     * @param value Brightness value (0-255)
     */
    void setBrightnessValue(uint8_t value);

    /**
     * @brief Sets the color mode.
     * @param mode Color mode:
     *        0 = Gradient (customizable via setGradientColors)
     *        1 = Spiral/Vortex (VRChat hypno style)
     *        2 = Plasma Effect (trippy interference patterns)
     *        3 = Radial Pulse (breathing effect from center)
     *        4 = Dual Spiral (customizable color and thickness)
     *        5 = Dual Circle (rotating concentric circles effect)
     */
    void setColorMode(uint8_t mode);

    /**
     * @brief Gets the current color mode.
     * @return Current color mode
     */
    uint8_t getColorMode() const;

    /**
     * @brief Sets the custom gradient colors.
     * @param topR Top color red component (0-255)
     * @param topG Top color green component (0-255)
     * @param topB Top color blue component (0-255)
     * @param bottomR Bottom color red component (0-255)
     * @param bottomG Bottom color green component (0-255)
     * @param bottomB Bottom color blue component (0-255)
     */
    void setGradientColors(uint8_t topR, uint8_t topG, uint8_t topB, uint8_t bottomR, uint8_t bottomG, uint8_t bottomB);

    /**
     * @brief Gets the top gradient color.
     * @param r Reference to red component (output)
     * @param g Reference to green component (output)
     * @param b Reference to blue component (output)
     */
    void getGradientTopColor(uint8_t& r, uint8_t& g, uint8_t& b) const;

    /**
     * @brief Gets the bottom gradient color.
     * @param r Reference to red component (output)
     * @param g Reference to green component (output)
     * @param b Reference to blue component (output)
     */
    void getGradientBottomColor(uint8_t& r, uint8_t& g, uint8_t& b) const;

    /**
     * @brief Sets the dual spiral color for Mode 4.
     * @param spiralR Spiral color red component (0-255)
     * @param spiralG Spiral color green component (0-255)
     * @param spiralB Spiral color blue component (0-255)
     */
    void setDualSpiralColor(uint8_t spiralR, uint8_t spiralG, uint8_t spiralB);

    /**
     * @brief Gets the dual spiral color.
     * @param r Reference to red component (output)
     * @param g Reference to green component (output)
     * @param b Reference to blue component (output)
     */
    void getDualSpiralColor(uint8_t& r, uint8_t& g, uint8_t& b) const;

    /**
     * @brief Sets the dual circle color for Mode 5.
     * @param circleR Circle color red component (0-255)
     * @param circleG Circle color green component (0-255)
     * @param circleB Circle color blue component (0-255)
     */
    void setDualCircleColor(uint8_t circleR, uint8_t circleG, uint8_t circleB);

    /**
     * @brief Gets the dual circle color.
     * @param r Reference to red component (output)
     * @param g Reference to green component (output)
     * @param b Reference to blue component (output)
     */
    void getDualCircleColor(uint8_t& r, uint8_t& g, uint8_t& b) const;

    /**
     * @brief Sets the effect thickness for Mode 4 & 5 (band width/spacing).
     * @param thickness Thickness value (0-255)
     */
    void setEffectThickness(uint8_t thickness);

    /**
     * @brief Gets the effect thickness.
     * @return Thickness value (0-255)
     */
    uint8_t getEffectThickness() const;

    /**
     * @brief Sets the effect rotation speed for Mode 4 & 5.
     * @param speed Speed value (0-255)
     */
    void setEffectSpeed(uint8_t speed);

    /**
     * @brief Gets the effect rotation speed.
     * @return Speed value (0-255)
     */
    uint8_t getEffectSpeed() const;

    // Drawing functions for various bitmaps
    /**
     * @brief Draws a bitmap fullscreen.
     * @param bitmap Bitmap array
     */
    void drawFullscreen(const uint8_t* bitmap);

    /**
     * @brief Draws an eye bitmap.
     * @param bitmap Bitmap array
     */
    void drawEye(const uint8_t* bitmap);

    /**
     * @brief Draws left and right eye bitmaps.
     * @param bitmapL Left eye bitmap array
     * @param bitmapR Right eye bitmap array
     */
    void drawEye(const uint8_t* bitmapL, const uint8_t* bitmapR);

    /**
     * @brief Draws an eye pupil bitmap at specified coordinates.
     * @param bitmap Bitmap array
     * @param x X coordinate
     * @param y Y coordinate
     */
    void drawEyePupil(const uint8_t* bitmap, int x, int y);

    /**
     * @brief Draws a nose bitmap.
     * @param bitmap Bitmap array
     */
    void drawNose(const uint8_t* bitmap);

    /**
     * @brief Draws a mouth bitmap.
     * @param bitmap Bitmap array
     */
    void drawMouth(const uint8_t* bitmap);

    /**
     * @brief Draws a color test pattern.
     */
    void drawColorTest();

    /**
     * @brief Draws a bitmap at specified coordinates.
     * @param bitmap Bitmap array
     * @param imageWidth Width of the bitmap
     * @param imageHeight Height of the bitmap
     * @param offsetX X offset
     * @param offsetY Y offset
     */
    void drawBitmap(const uint8_t* bitmap, int imageWidth, int imageHeight, int offsetX, int offsetY);

    /**
     * @brief Draws left and right bitmaps at specified coordinates.
     * @param bitmapL Left bitmap array
     * @param bitmapR Right bitmap array
     * @param imageWidth Width of the bitmaps
     * @param imageHeight Height of the bitmaps
     * @param offsetX X offset
     * @param offsetY Y offset
     */
    void drawBitmap(const uint8_t* bitmapL, const uint8_t* bitmapR, int imageWidth, int imageHeight, int offsetX, int offsetY);

    /**
     * @brief Draws a bitmap with specified color at specified coordinates.
     * @param bitmap Bitmap array
     * @param imageWidth Width of the bitmap
     * @param imageHeight Height of the bitmap
     * @param offsetX X offset
     * @param offsetY Y offset
     * @param r Red component (0-255)
     * @param g Green component (0-255)
     * @param b Blue component (0-255)
     */
    void drawBitmap(const uint8_t* bitmap, int imageWidth, int imageHeight, int offsetX, int offsetY, uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Draws an RLE-encoded bitmap at specified coordinates.
     * @param rleBitmap RLE-encoded bitmap array
     * @param imageWidth Width of the bitmap
     * @param imageHeight Height of the bitmap
     * @param offsetX X offset
     * @param offsetY Y offset
     */
    void drawBitmapRLE(const uint8_t* rleBitmap, int imageWidth, int imageHeight, int offsetX, int offsetY);

    /**
     * @brief Draws an RLE-encoded bitmap with specified color at specified coordinates.
     * @param rleBitmap RLE-encoded bitmap array
     * @param imageWidth Width of the bitmap
     * @param imageHeight Height of the bitmap
     * @param offsetX X offset
     * @param offsetY Y offset
     * @param r Red component (0-255)
     * @param g Green component (0-255)
     * @param b Blue component (0-255)
     */
    void drawBitmapRLE(const uint8_t* rleBitmap, int imageWidth, int imageHeight, int offsetX, int offsetY, uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Draws separate left and right RLE-encoded bitmaps at specified coordinates.
     * @param rleBitmapL Left RLE-encoded bitmap array
     * @param rleBitmapR Right RLE-encoded bitmap array
     * @param imageWidth Width of the bitmaps
     * @param imageHeight Height of the bitmaps
     * @param offsetX X offset
     * @param offsetY Y offset
     */
    void drawBitmapRLE(const uint8_t* rleBitmapL, const uint8_t* rleBitmapR, int imageWidth, int imageHeight, int offsetX, int offsetY);
};
