#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "Bitmaps/accColor.h"

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

#define PANEL_RES_X 64
#define PANEL_RES_Y 32
#define PANELS_NUMBER 2
#define SCREEN_WIDTH PANEL_RES_X* PANELS_NUMBER
#define SCREEN_HEIGHT PANEL_RES_Y

// Define the number of frames and the transition duration
#define NUM_FRAMES 4
#define INTERPOLATION_FACTOR 5
#define FRAME_RATE 60

class LEDMatrixDisplay {
   private:
    MatrixPanel_I2S_DMA* matrix;
    const int panelWidth = PANEL_RES_X,
              panelHeight = PANEL_RES_Y;
    uint8_t panelBrightness = 64;
    int eyeWidth = 32, eyeHeight = 18;

   public:
    LEDMatrixDisplay() {
        HUB75_I2S_CFG mxconfig(panelWidth, panelHeight, PANELS_NUMBER);
        mxconfig.double_buff = false;  // Turn of double buffer
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
    void drawBitmap(const uint8_t* bitmap, int imageWidth, int imageHeight, int offsetX, int offsetY, bool drawBlack = true) {
        uint8_t r, g, b;
        for (int i = 0; i < imageHeight; i++) {
            for (int j = 0, j2 = panelWidth - 1; j < imageWidth; j++, j2--) {
                uint8_t pixel = pgm_read_byte(bitmap + i * imageWidth + j);  // read the bytes from program memory
                getColorMap(pixel, i + offsetY, r, g, b);
                // getColorWave(pixel, i, j + offsetY, r, g, b);
                if (drawBlack || (r != 0 || g != 0 || b != 0)) {
                    matrix->drawPixelRGB888(offsetX + j, offsetY + i, r, g, b);
                    matrix->drawPixelRGB888(-offsetX + panelWidth + j2, offsetY + i, r, g, b);
                }
            }
        }
    }

    void drawFullscreen(const uint8_t* bitmap) {
        drawBitmap(bitmap, panelWidth, panelHeight, 0, 0);
    }

    // uint8_t* prevEyeFrame;
    const uint8_t* prevEyeFrame = new uint8_t[eyeWidth * eyeHeight];
    void drawEye(const uint8_t* bitmap) {
        // drawBitmap(bitmap, eyeWidth, eyeHeight, 12, 0);
        transitionFrames2(prevEyeFrame, bitmap, eyeWidth, eyeHeight);
        // copyFrame(prevEyeFrame, bitmap, eyeWidth, eyeHeight);
        prevEyeFrame = bitmap;
    }

    void drawEyePupil(const uint8_t* bitmap, int x, int y) {
        drawBitmap(bitmap, 6, 6, 14 + x, 5 + y, false);
    }

    void drawNose(const uint8_t* bitmap) {
        drawBitmap(bitmap, 10, 6, 54, 6);
    }

    void drawMouth(const uint8_t* bitmap) {
        drawBitmap(bitmap, 50, 14, 14, 18);
    }

    void drawColorTest() {
        uint8_t r, g, b;
        for (int i = 0; i < 64; i++) {
            getColorMap(255, i, r, g, b);
            matrix->drawPixelRGB888(0, i, r, g, b);
            matrix->drawPixelRGB888(SCREEN_WIDTH - 1, i, r, g, b);
        }
    }

    //
    unsigned long previousFrameTime;
    unsigned long frameDelay = 1000 / FRAME_RATE;
    // State variables for frame interpolation
    bool isInterpolating = false;
    int interpolationIndex = 0;
    void transitionFrames2(const uint8_t* currentFrame, const uint8_t* nextFrame, int width, int height) {
        unsigned long currentMillis = millis();
        // Check if it's time to update the frame
        if (currentMillis - previousFrameTime >= frameDelay) {
            previousFrameTime = currentMillis;
            // Display current frame
            drawBitmap(currentFrame, width, height, 12, 0);
            // Start interpolation if needed
            if (!isInterpolating) {
                startInterpolation();
            }
        }

        // Update interpolation frames
        if (isInterpolating) {
            if (updateInterpolation(currentFrame, nextFrame, width, height)) {
                isInterpolating = false;
                // Shift nextFrame data to currentFrame for the next iteration
                currentFrame = nextFrame;
            }
        }
    }
    void startInterpolation() {
        interpolationIndex = 0;
        isInterpolating = true;
    }
    bool updateInterpolation(const uint8_t* currentFrame, const uint8_t* nextFrame, int width, int height) {
        uint8_t interpolatedFrame[width * height];
        interpolateFrames(currentFrame, nextFrame, interpolatedFrame, interpolationIndex, INTERPOLATION_FACTOR, width, height);
        // matrix.drawBitmap(0, 0, currentFrame, MATRIX_WIDTH, MATRIX_HEIGHT, LED_ON);
        drawBitmap(interpolatedFrame, width, height, 12, 0);
        interpolationIndex++;
        // Check if interpolation is complete
        return (interpolationIndex >= INTERPOLATION_FACTOR);
    }
    void interpolateFrames(const uint8_t* current, const uint8_t* next, uint8_t* interpolated, int index, int totalFrames, int width, int height) {
        for (int i = 0; i < width * height; i++) {
            // Perform linear interpolation for each pixel
            uint8_t currentPixel = current[i];
            uint8_t nextPixel = next[i];
            uint8_t interpolatedPixel = currentPixel + ((nextPixel - currentPixel) * index) / totalFrames;
            interpolated[i] = interpolatedPixel;
        }
    }

    // void transitionFrames(const uint8_t* frameA, const uint8_t* frameB, int width, int height) {
    //     if (isSameFrame(frameA, frameB, width, height)) {
    //         // Render the current frame directly without any transition
    //         drawBitmap(frameB, width, height, 12, 0);
    //     } else {
    //         Serial.println("**************** Not Same face");
    //         for (int step = 0; step <= NUM_FRAMES; step++) {
    //             // Calculate the interpolation factor (0 to 1)
    //             float t = float(step) / NUM_FRAMES;
    //             Serial.print("\\\\\\\\\\\\\\\\\\\\\\\\\\");
    //             Serial.println(t);
    //             // Create an intermediate frame with motion blur effect
    //             uint8_t* intermediateFrame = new uint8_t[width * height];
    //             if (t == 0) {
    //                 // intermediateFrame = frameA;
    //                 memcpy(intermediateFrame, frameA, width * height);
    //             } else if (t == 1) {
    //                 // intermediateFrame = frameB;
    //                 memcpy(intermediateFrame, frameB, width * height);
    //             } else {
    //                 // Serial.println("******************************* Draw interpolateFrames");
    //                 interpolateFrames(frameA, frameB, intermediateFrame, width, height, t);
    //             }
    //             // Render the intermediate frame on the matrix
    //             drawBitmap(intermediateFrame, width, height, 12, 0);
    //             // Deallocate memory for intermediateFrame
    //             delete[] intermediateFrame;
    //         }
    //     }
    // }

    // Function to check if two frames are the same
    bool isSameFrame(const uint8_t* frameA, const uint8_t* frameB, int width, int height) {
        for (int i = 0; i < width * height; i++) {
            if (frameA[i] != frameB[i]) {
                return false;
            }
        }
        return true;
    }

    // Function to interpolate pixel values between two frames
    // void interpolateFrames(const uint8_t* frameA, const uint8_t* frameB, uint8_t* intermediateFrame, int width, int height, float t) {
    //     for (int i = 0; i < width * height; i++) {
    //         intermediateFrame[i] = interpolatePixels(frameA[i], frameB[i], t);
    //     }
    // }

    // Linear interpolation function
    uint8_t lerp(uint8_t start, uint8_t end, float t) {
        return start + static_cast<uint8_t>((end - start) * t);
    }

    uint8_t interpolatePixels(uint8_t pixel1, uint8_t pixel2, float t) {
        // Perform linear interpolation
        return (1 - t) * pixel1 + t * pixel2;
    }
};