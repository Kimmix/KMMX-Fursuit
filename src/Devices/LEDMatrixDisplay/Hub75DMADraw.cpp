#include "Hub75DMA.h"

void Hub75DMA::drawFullscreen(const uint8_t* bitmap) {
    drawBitmap(bitmap, panelWidth, panelHeight, 0, 0);
}

void Hub75DMA::drawEye(const uint8_t* bitmap) {
    drawBitmap(bitmap, eyeWidth, eyeHeight, 7, 0);
}

void Hub75DMA::drawEye(const uint8_t* bitmapL, const uint8_t* bitmapR) {
    drawBitmap(bitmapL, bitmapR, eyeWidth, eyeHeight, 7, 0);
}

void Hub75DMA::drawEyePupil(const uint8_t* bitmap, int x, int y) {
    drawBitmap(bitmap, 6, 6, 21 + x, 5 + y);
}

void Hub75DMA::drawNose(const uint8_t* bitmap) {
    drawBitmap(bitmap, 8, 5, 56, 7);
}

void Hub75DMA::drawMouth(const uint8_t* bitmap) {
    drawBitmap(bitmap, mouthWidth, mouthHeight, 14, 18);
}

void Hub75DMA::drawColorTest() {
    uint8_t r, g, b;
    for (int i = 0; i < 64; i++) {
        getColorMap(255, i, r, g, b);
        matrix->drawPixelRGB888(SCREEN_WIDTH - 1, i, r, g, b);
        matrix->drawPixelRGB888(SCREEN_WIDTH - 2, i, r, g, b);
        matrix->drawPixelRGB888(SCREEN_WIDTH - 3, i, r, g, b);
    }
}

void Hub75DMA::drawBitmap(const uint8_t* bitmap, int imageWidth, int imageHeight, int offsetX, int offsetY) {
    for (int i = 0; i < imageHeight; i++) {
        int offsetYPlusI = offsetY + i;
        for (int j = 0, j2 = panelWidth - 1; j < imageWidth; j++, j2--) {
            uint8_t pixel = pgm_read_byte(bitmap + i * imageWidth + j);
            if (pixel > 30) {
                uint8_t r, g, b;
                getColorMap(pixel, i + offsetY, r, g, b);
                matrix->drawPixelRGB888(offsetX + j, offsetYPlusI, r, g, b);
                matrix->drawPixelRGB888(-offsetX + panelWidth + j2, offsetYPlusI, r, g, b);
            }
        }
    }
}

void Hub75DMA::drawBitmap(const uint8_t* bitmapL, const uint8_t* bitmapR, int imageWidth, int imageHeight, int offsetX, int offsetY) {
    for (int i = 0; i < imageHeight; i++) {
        int offsetYPlusI = offsetY + i;
        for (int j = 0, j2 = panelWidth - 1; j < imageWidth; j++, j2--) {
            uint8_t pixelL = pgm_read_byte(bitmapL + i * imageWidth + j);
            uint8_t pixelR = pgm_read_byte(bitmapR + i * imageWidth + j);
            if (pixelL > 30) {
                uint8_t r, g, b;
                getColorMap(pixelL, i + offsetY, r, g, b);
                matrix->drawPixelRGB888(offsetX + j, offsetYPlusI, r, g, b);
            }
            if (pixelR > 30) {
                uint8_t r, g, b;
                getColorMap(pixelR, i + offsetY, r, g, b);
                matrix->drawPixelRGB888(-offsetX + panelWidth + j2, offsetYPlusI, r, g, b);
            }
        }
    }
}

void Hub75DMA::drawBitmap(const uint8_t* bitmap, int imageWidth, int imageHeight, int offsetX, int offsetY, uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < imageHeight; i++) {
        int offsetYPlusI = offsetY + i;
        for (int j = 0, j2 = panelWidth - 1; j < imageWidth; j++, j2--) {
            uint8_t pixel = pgm_read_byte(bitmap + i * imageWidth + j);
            if (pixel > 30) {
                matrix->drawPixelRGB888(offsetX + j, offsetYPlusI, r, g, b);
            }
        }
    }
}
