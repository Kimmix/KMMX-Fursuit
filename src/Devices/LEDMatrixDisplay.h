#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "Bitmaps/accColor.h"

#define PANEL_RES_X 64
#define PANEL_RES_Y 32
#define PANELS_NUMBER 2
#define SCREEN_WIDTH PANEL_RES_X* PANELS_NUMBER
#define SCREEN_HEIGHT PANEL_RES_Y

// Define the number of frames and the transition duration
#define INTERPOLATION_FACTOR 4
#define INTERPOLATION_DURATION 5

class LEDMatrixDisplay {
   private:
    MatrixPanel_I2S_DMA* matrix;
    const int panelWidth = PANEL_RES_X,
              panelHeight = PANEL_RES_Y;
    uint8_t panelBrightness = 40;
    int eyeWidth = 32, eyeHeight = 18,
        mouthWidth = 50, mouthHeight = 14;

   public:
    LEDMatrixDisplay() {
        HUB75_I2S_CFG mxconfig(panelWidth, panelHeight, PANELS_NUMBER);
        mxconfig.double_buff = true;  // Turn of double buffer
        // mxconfig.clkphase = true;
        matrix = new MatrixPanel_I2S_DMA(mxconfig);
        if (!matrix->begin())
            Serial.println("****** I2S memory allocation failed ***********");
        matrix->setBrightness8(panelBrightness);
        matrix->clearScreen();
    }

    void render() {
        matrix->flipDMABuffer();
    }

    void clearScreen() {
        matrix->clearScreen();
    }

    int getBrightnessValue() {
        return panelBrightness;
    }

    int getResX() {
        return PANEL_RES_X;
    }

    uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
        return matrix->color565(r, g, b);
    }

    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
        matrix->drawRect(x, y, w, h, color);
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color) {
        matrix->drawPixel(x, y, color);
    }

    void setTextColor(uint16_t color) {
        matrix->setTextColor(color);
    }

    void setCursor(int16_t x, int16_t y) {
        matrix->setCursor(x, y);
    }

    void print(char str[]) {
        matrix->print(str);
    }

    void print(int n) {
        matrix->print(n);
    }

    void getTextBounds(const String& str, int16_t x, int16_t y, int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h) {
        matrix->getTextBounds(str, x, y, x1, y1, w, h);
    }

    void setBrightnessValue(uint8_t value) {
        // Ensure that the brightness value is between 0 and 255
        if (value < 0) {
            value = 0;
        } else if (value > 255) {
            value = 255;
        }
        panelBrightness = value;
        matrix->setBrightness8(panelBrightness);
    }

    /**
     * @brief Returns the RGB value of a color in a gradient
     * @param lightness Brightness of color (8-bit)
     * @param row Current row of the LED matrix
     * @param uint8_t r - RGB888 color
     * @param uint8_t g - RGB888 color
     * @param uint8_t b - RGB888 color
     */
    void getColorMap(const uint8_t lightness, const int row, uint8_t& r, uint8_t& g, uint8_t& b) {
        const int index = row * 3;
        if (row >= 0 && index < sizeof(accColor)) {  // add a check for valid range
            uint16_t factor = lightness << 8;        // multiply by 256 (i.e., 2^8) using bit shift
            r = (factor * accColor[index]) >> 16;    // divide by 65536 (i.e., 2^16) using bit shift
            g = (factor * accColor[index + 1]) >> 16;
            b = (factor * accColor[index + 2]) >> 16;
        } else {
            // handle the case where row is out of range
            r = g = b = 0;
        }
    }

    void getBlackWhiteWave(const uint8_t brightness, const int row, const int col, uint8_t& r, uint8_t& g, uint8_t& b) {
        // Compute the black and white wave pattern based on the brightness, row, col, and time inputs
        double wave = (row + col) * 0.1 + millis() * 0.008;
        double gray = sin(wave) * 127 + 128;
        r = g = b = static_cast<uint8_t>(gray);

        // Scale the gray value based on the brightness input
        double scale = brightness / 255.0;
        gray = gray * scale;

        // Constrain the gray value to the range [0, 255]
        r = g = b = constrain(static_cast<uint8_t>(gray), 0, 255);
    }

    void getColorWave(const uint8_t brightness, const int row, const int col, uint8_t& r, uint8_t& g, uint8_t& b) {
        // Compute the color1 and color2 wave pattern based on the brightness, row, col, and time inputs
        double wave = (row + col) * 0.4 + millis() * 0.01;
        double color1 = sin(wave) * 127 + 128;
        double color2 = sin(wave + PI) * 127 + 128;

        // Scale the color1 and color2 values based on the brightness input
        double scale = brightness / 255.0;
        color1 *= scale;
        color2 *= scale;

        // Constrain the color1 and color2 values to the range [0, 255]
        r = constrain(static_cast<uint8_t>(color2), 0, 255);
        g = constrain(static_cast<uint8_t>(color1), 0, 255);
        b = constrain(static_cast<uint8_t>(color1 + color2), 0, 255);
    }

    /**
     * @brief Draws a bitmap onto the LED matrix
     * @param bitmap Array of 8-bit values representing the image
     * @param image_width Width of the image in pixels
     * @param image_height Height of the image in pixels
     * @param offset_x X offset of the image
     * @param offset_y Y offset of the image
     */
    void drawBitmap(const uint8_t* bitmap, int imageWidth, int imageHeight, int offsetX, int offsetY) {
        uint8_t r, g, b;
        for (int i = 0; i < imageHeight; i++) {
            int offsetYPlusI = offsetY + i;
            for (int j = 0, j2 = panelWidth - 1; j < imageWidth; j++, j2--) {
                uint8_t pixel = pgm_read_byte(bitmap + i * imageWidth + j);  // read the bytes from program memory
                if (pixel <= 30) {
                    continue;
                }
                getColorMap(pixel, i + offsetY, r, g, b);
                // getBlackWhiteWave(pixel, i, j + offsetY, r, g, b);
                matrix->drawPixelRGB888(offsetX + j, offsetYPlusI, r, g, b);
                matrix->drawPixelRGB888(-offsetX + panelWidth + j2, offsetYPlusI, r, g, b);
            }
        }
    }

    void drawBitmap(const uint8_t* bitmap, int imageWidth, int imageHeight, int offsetX, int offsetY, uint8_t r, uint8_t g, uint8_t b) {
        for (int i = 0; i < imageHeight; i++) {
            int offsetYPlusI = offsetY + i;
            for (int j = 0, j2 = panelWidth - 1; j < imageWidth; j++, j2--) {
                matrix->drawPixelRGB888(offsetX + j, offsetYPlusI, r, g, b);
                matrix->drawPixelRGB888(-offsetX + panelWidth + j2, offsetYPlusI, r, g, b);
            }
        }
    }

    void drawFullscreen(const uint8_t* bitmap) {
        drawBitmap(bitmap, panelWidth, panelHeight, 0, 0);
    }

    uint8_t* prevEyeFrame = new uint8_t[eyeWidth * eyeHeight];
    void drawEye(const uint8_t* bitmap) {
        // drawBitmap(bitmap, eyeWidth, eyeHeight, 12, 0);
        transitionFrames(prevEyeFrame, bitmap, eyeWidth, eyeHeight, 12, 0);
        memcpy(prevEyeFrame, bitmap, 12 * 0);
    }

    void drawEyePupil(const uint8_t* bitmap, int x, int y) {
        drawBitmap(bitmap, 6, 6, 21 + x, 5 + y);
    }

    void drawNose(const uint8_t* bitmap) {
        drawBitmap(bitmap, 8, 5, 56, 5);
    }

    uint8_t* prevMouthFrame = new uint8_t[mouthWidth * mouthHeight];
    void drawMouth(const uint8_t* bitmap) {
        // drawBitmap(bitmap, mouthWidth, mouthHeight, 14, 18);
        transitionFrames(prevMouthFrame, bitmap, mouthWidth, mouthHeight, 14, 18);
        memcpy(prevMouthFrame, bitmap, mouthWidth * mouthHeight);
    }

    void drawColorTest() {
        uint8_t r, g, b;
        for (int i = 0; i < 64; i++) {
            getColorMap(255, i, r, g, b);
            // matrix->drawPixelRGB888(0, i, r, g, b);
            matrix->drawPixelRGB888(SCREEN_WIDTH - 1, i, r, g, b);
            matrix->drawPixelRGB888(SCREEN_WIDTH - 2, i, r, g, b);
            matrix->drawPixelRGB888(SCREEN_WIDTH - 3, i, r, g, b);
        }
    }

    unsigned long previousFrameTime;
    unsigned long frameDelay = INTERPOLATION_DURATION;
    // State variables for frame interpolation
    bool isInterpolating = false;
    int interpolationIndex = 0;
    void transitionFrames(uint8_t* currentFrame, const uint8_t* nextFrame, int width, int height, int offsetX, int offsetY) {
        // Start interpolation if needed
        if (!isInterpolating && !isFrameSame(currentFrame, nextFrame, width, height)) {
            startInterpolation();
        }
        // Update interpolation frames
        uint8_t interpolatedFrame[width * height];
        memcpy(interpolatedFrame, currentFrame, width * height);

        if (isInterpolating) {
            unsigned long currentTime = millis();
            if (currentTime - previousFrameTime >= frameDelay) {
                if (interpolationIndex < INTERPOLATION_FACTOR) {
                    interpolateFrames(currentFrame, nextFrame, interpolatedFrame, interpolationIndex, INTERPOLATION_FACTOR, width, height);
                    interpolationIndex++;
                } else {
                    isInterpolating = false;
                    memcpy(currentFrame, nextFrame, width * height);
                }
            }
            // Serial.print(interpolationIndex);
            drawBitmap(interpolatedFrame, width, height, offsetX, offsetY);
        } else {
            drawBitmap(currentFrame, width, height, offsetX, offsetY);
        }
    }

    void startInterpolation() {
        // Serial.println();
        // Serial.println("startInterpolation");
        interpolationIndex = 0;
        isInterpolating = true;
    }

    void interpolateFrames(const uint8_t* current, const uint8_t* next, uint8_t* interpolated, int index, int totalFrames, int width, int height) {
        for (int i = 0; i < width * height; i++) {
            uint8_t currentPixel = current[i];
            uint8_t nextPixel = next[i];

            // Perform linear interpolation for each pixel
            uint8_t interpolatedPixel = currentPixel + ((nextPixel - currentPixel) * index) / totalFrames;
            interpolated[i] = interpolatedPixel;

            // Cosine Interpolation
            // float t = (float)index / totalFrames;
            // float interpolatedValue = currentPixel + (1 - cos(t * M_PI)) * (nextPixel - currentPixel) / 2;
            // interpolated[i] = static_cast<uint8_t>(interpolatedValue);

            // Cross blend the pixel values
            // uint8_t interpolatedPixel = (currentPixel * (totalFrames - index) + nextPixel * index) / totalFrames;
            // interpolated[i] = interpolatedPixel;
        }
    }
    bool isFrameSame(const uint8_t* frame1, const uint8_t* frame2, int width, int height) {
        for (int i = 0; i < width * height; i++) {
            if (frame1[i] != frame2[i]) {
                return false;
            }
        }
        return true;
    }
};