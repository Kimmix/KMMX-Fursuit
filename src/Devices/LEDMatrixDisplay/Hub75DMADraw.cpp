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
#if defined(USE_RLE_BITMAPS) && USE_RLE_BITMAPS
    drawBitmapRLE(bitmap, eyeWidth, eyeHeight, eyeOffsetX, eyeOffsetY);
#else
    drawBitmap(bitmap, eyeWidth, eyeHeight, eyeOffsetX, eyeOffsetY);
#endif
}

void Hub75DMA::drawEye(const uint8_t* bitmapL, const uint8_t* bitmapR) {
#if defined(USE_RLE_BITMAPS) && USE_RLE_BITMAPS
    drawBitmapRLE(bitmapL, bitmapR, eyeWidth, eyeHeight, eyeOffsetX, eyeOffsetY);
#else
    drawBitmap(bitmapL, bitmapR, eyeWidth, eyeHeight, eyeOffsetX, eyeOffsetY);
#endif
}

void Hub75DMA::drawMouth(const uint8_t* bitmap) {
#if defined(USE_RLE_BITMAPS) && USE_RLE_BITMAPS
    drawBitmapRLE(bitmap, mouthWidth, mouthHeight, mouthOffsetX, mouthOffsetY);
#else
    drawBitmap(bitmap, mouthWidth, mouthHeight, mouthOffsetX, mouthOffsetY);
#endif
}

void Hub75DMA::drawColorTest() {
    uint8_t r, g, b;
    for (int i = 0; i < 64; i++) {
        getColorMap(255, i, r, g, b);
        matrix->drawPixelRGB888(screenWidth - 1, i, r, g, b);
        matrix->drawPixelRGB888(screenWidth - 2, i, r, g, b);
        matrix->drawPixelRGB888(screenWidth - 3, i, r, g, b);
    }
}

void Hub75DMA::drawBitmap(const uint8_t* bitmap, int imageWidth, int imageHeight, int offsetX, int offsetY) {
    for (int i = 0; i < imageHeight; i++) {
        int offsetYPlusI = offsetY + i;
        for (int j = 0, j2 = panelWidth - 1; j < imageWidth; j++, j2--) {
            uint8_t pixel = pgm_read_byte(bitmap + i * imageWidth + j);
            if (pixel > minimumPixelBrightness) {
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
            if (pixelL > minimumPixelBrightness) {
                uint8_t r, g, b;
                getColorMap(pixelL, i + offsetY, r, g, b);
                matrix->drawPixelRGB888(offsetX + j, offsetYPlusI, r, g, b);
            }
            if (pixelR > minimumPixelBrightness) {
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
            if (pixel > minimumPixelBrightness) {
                matrix->drawPixelRGB888(offsetX + j, offsetYPlusI, r, g, b);
            }
        }
    }
}

void Hub75DMA::drawBitmapRLE(const uint8_t* rleBitmap, int imageWidth, int imageHeight, int offsetX, int offsetY) {
    int i = 0, x = 0, y = 0;
    while (y < imageHeight) {
        uint8_t count = pgm_read_byte(rleBitmap + i++);
        uint8_t value = pgm_read_byte(rleBitmap + i++);
        for (uint8_t c = 0; c < count; c++) {
            if (value > minimumPixelBrightness) {
                uint8_t r, g, b;
                getColorMap(value, y + offsetY, r, g, b);
                // Draw left panel
                matrix->drawPixelRGB888(offsetX + x, offsetY + y, r, g, b);
                // Draw right panel (mirrored, same width offset as normal drawBitmap)
                matrix->drawPixelRGB888(-offsetX + panelWidth * 2 - 1 - x, offsetY + y, r, g, b);
            }
            x++;
            if (x >= imageWidth) {
                x = 0;
                y++;
            }
            if (y >= imageHeight) break;
        }
    }
}

void Hub75DMA::drawBitmapRLE(const uint8_t* rleBitmap, int imageWidth, int imageHeight, int offsetX, int offsetY, uint8_t r, uint8_t g, uint8_t b) {
    int i = 0;
    for (int row = 0; row < imageHeight; row++) {
        int offsetYPlusRow = offsetY + row;
        int col = 0;
        while (col < imageWidth) {
            uint8_t count = pgm_read_byte(rleBitmap + i++);
            uint8_t value = pgm_read_byte(rleBitmap + i++);
            for (uint8_t c = 0; c < count && col < imageWidth; c++, col++) {
                if (value > minimumPixelBrightness) {
                    matrix->drawPixelRGB888(offsetX + col, offsetYPlusRow, r, g, b);
                }
            }
        }
    }
}

void Hub75DMA::drawBitmapRLE(const uint8_t* rleBitmapL, const uint8_t* rleBitmapR, int imageWidth, int imageHeight, int offsetX, int offsetY) {
    // Process left and right bitmaps separately

    // Draw left bitmap
    int iL = 0;
    int xL = 0, yL = 0;
    while (yL < imageHeight) {
        uint8_t countL = pgm_read_byte(rleBitmapL + iL++);
        uint8_t valueL = pgm_read_byte(rleBitmapL + iL++);
        for (uint8_t c = 0; c < countL; c++) {
            if (valueL > minimumPixelBrightness) {
                uint8_t r, g, b;
                getColorMap(valueL, yL + offsetY, r, g, b);
                matrix->drawPixelRGB888(offsetX + xL, offsetY + yL, r, g, b);
            }
            xL++;
            if (xL >= imageWidth) {
                xL = 0;
                yL++;
            }
            if (yL >= imageHeight) break;
        }
    }

    // Draw right bitmap
    int iR = 0;
    int xR = 0, yR = 0;
    while (yR < imageHeight) {
        uint8_t countR = pgm_read_byte(rleBitmapR + iR++);
        uint8_t valueR = pgm_read_byte(rleBitmapR + iR++);
        for (uint8_t c = 0; c < countR; c++) {
            if (valueR > minimumPixelBrightness) {
                uint8_t r, g, b;
                getColorMap(valueR, yR + offsetY, r, g, b);
                matrix->drawPixelRGB888(-offsetX + panelWidth * 2 - 1 - xR, offsetY + yR, r, g, b);
            }
            xR++;
            if (xR >= imageWidth) {
                xR = 0;
                yR++;
            }
            if (yR >= imageHeight) break;
        }
    }
}
