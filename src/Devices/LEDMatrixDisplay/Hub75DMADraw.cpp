#include "Hub75DMA.h"
#include "config.h"

void Hub75DMA::drawFullscreen(const uint8_t* bitmap) {
    drawBitmap(bitmap, panelWidth, panelHeight, 0, 0);
}

void Hub75DMA::drawEyePupil(const uint8_t* bitmap, int x, int y) {
    drawBitmap(bitmap, googlyPupilWidth, googlyPupilHeight, googlyPupilOffsetX + x, googlyPupilOffsetY + y);
}

void Hub75DMA::drawNose(const uint8_t* bitmap) {
    drawBitmap(bitmap, noseWidth, noseHeight, noseOffsetX, noseOffsetY);
}

void Hub75DMA::drawEye(const uint8_t* bitmap) {
    lastEyeBitmap = bitmap;  // Track last drawn eye bitmap
    drawBitmap(bitmap, eyeWidth, eyeHeight, eyeOffsetX, eyeOffsetY);
}

void Hub75DMA::drawEye(const uint8_t* bitmapL, const uint8_t* bitmapR) {
    lastEyeBitmap = bitmapL;  // Track left eye bitmap (for mirroring)
    drawBitmap(bitmapL, bitmapR, eyeWidth, eyeHeight, eyeOffsetX, eyeOffsetY);
}

void Hub75DMA::drawMouth(const uint8_t* bitmap) {
    lastMouthBitmap = bitmap;  // Track last drawn mouth bitmap
    drawBitmap(bitmap, mouthWidth, mouthHeight, mouthOffsetX, mouthOffsetY);
}

void Hub75DMA::drawColorTest() {
    uint8_t r, g, b;
    for (int i = 0; i < 64; i++) {
        getColorMap(255, i, screenWidth - 1, r, g, b);
        matrix->drawPixelRGB888(screenWidth - 1, i, r, g, b);
        getColorMap(255, i, screenWidth - 2, r, g, b);
        matrix->drawPixelRGB888(screenWidth - 2, i, r, g, b);
        getColorMap(255, i, screenWidth - 3, r, g, b);
        matrix->drawPixelRGB888(screenWidth - 3, i, r, g, b);
    }
}

void Hub75DMA::drawBitmap(const uint8_t* bitmap, int imageWidth, int imageHeight, int offsetX, int offsetY) {
    for (int i = 0; i < imageHeight; i++) {
        const int offsetYPlusI = offsetY + i;
        const uint8_t* rowPtr = bitmap + i * imageWidth;  // Hoist row offset calculation
        const int mirrorBaseX = -offsetX + panelWidth;

        for (int j = 0; j < imageWidth; j++) {
            const uint8_t pixel = pgm_read_byte(rowPtr + j);
            if (pixel > minimumPixelBrightness) {
                uint8_t r, g, b;
                const int colLeft = offsetX + j;
                const int colRight = mirrorBaseX + (panelWidth - 1 - j);
                getColorMap(pixel, offsetYPlusI, colLeft, r, g, b);
                matrix->drawPixelRGB888(colLeft, offsetYPlusI, r, g, b);
                getColorMap(pixel, offsetYPlusI, colRight, r, g, b);
                matrix->drawPixelRGB888(colRight, offsetYPlusI, r, g, b);
            }
        }
    }
}

void Hub75DMA::drawBitmap(const uint8_t* bitmapL, const uint8_t* bitmapR, int imageWidth, int imageHeight, int offsetX, int offsetY) {
    for (int i = 0; i < imageHeight; i++) {
        const int offsetYPlusI = offsetY + i;
        const uint8_t* rowPtrL = bitmapL + i * imageWidth;  // Hoist row offset calculation
        const uint8_t* rowPtrR = bitmapR + i * imageWidth;
        const int mirrorBaseX = -offsetX + panelWidth;

        for (int j = 0; j < imageWidth; j++) {
            const uint8_t pixelL = pgm_read_byte(rowPtrL + j);
            const uint8_t pixelR = pgm_read_byte(rowPtrR + j);

            if (pixelL > minimumPixelBrightness) {
                uint8_t r, g, b;
                const int colLeft = offsetX + j;
                getColorMap(pixelL, offsetYPlusI, colLeft, r, g, b);
                matrix->drawPixelRGB888(colLeft, offsetYPlusI, r, g, b);
            }
            if (pixelR > minimumPixelBrightness) {
                uint8_t r, g, b;
                const int colRight = mirrorBaseX + (panelWidth - 1 - j);
                getColorMap(pixelR, offsetYPlusI, colRight, r, g, b);
                matrix->drawPixelRGB888(colRight, offsetYPlusI, r, g, b);
            }
        }
    }
}

void Hub75DMA::drawBitmap(const uint8_t* bitmap, int imageWidth, int imageHeight, int offsetX, int offsetY, uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < imageHeight; i++) {
        const int offsetYPlusI = offsetY + i;
        const uint8_t* rowPtr = bitmap + i * imageWidth;  // Hoist row offset calculation

        for (int j = 0; j < imageWidth; j++) {
            const uint8_t pixel = pgm_read_byte(rowPtr + j);
            if (pixel > minimumPixelBrightness) {
                matrix->drawPixelRGB888(offsetX + j, offsetYPlusI, r, g, b);
            }
        }
    }
}


