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
        const int offsetYPlusI = offsetY + i;
        const uint8_t* rowPtr = bitmap + i * imageWidth;  // Hoist row offset calculation
        const int mirrorBaseX = -offsetX + panelWidth;

        for (int j = 0; j < imageWidth; j++) {
            const uint8_t pixel = pgm_read_byte(rowPtr + j);
            if (pixel > minimumPixelBrightness) {
                uint8_t r, g, b;
                getColorMap(pixel, offsetYPlusI, r, g, b);
                matrix->drawPixelRGB888(offsetX + j, offsetYPlusI, r, g, b);
                matrix->drawPixelRGB888(mirrorBaseX + (panelWidth - 1 - j), offsetYPlusI, r, g, b);
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
                getColorMap(pixelL, offsetYPlusI, r, g, b);
                matrix->drawPixelRGB888(offsetX + j, offsetYPlusI, r, g, b);
            }
            if (pixelR > minimumPixelBrightness) {
                uint8_t r, g, b;
                getColorMap(pixelR, offsetYPlusI, r, g, b);
                matrix->drawPixelRGB888(mirrorBaseX + (panelWidth - 1 - j), offsetYPlusI, r, g, b);
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

void Hub75DMA::drawBitmapRLE(const uint8_t* rleBitmap, int imageWidth, int imageHeight, int offsetX, int offsetY) {
    int i = 0, x = 0, y = 0;
    const int mirrorBaseX = -offsetX + panelWidth;  // Consistent with drawBitmap

    while (y < imageHeight) {
        const uint8_t count = pgm_read_byte(rleBitmap + i++);
        const uint8_t value = pgm_read_byte(rleBitmap + i++);

        if (value > minimumPixelBrightness) {
            uint8_t r, g, b;
            getColorMap(value, y + offsetY, r, g, b);

            for (uint8_t c = 0; c < count; c++) {
                // Draw left panel
                matrix->drawPixelRGB888(offsetX + x, offsetY + y, r, g, b);
                // Draw right panel (mirrored, consistent with drawBitmap)
                matrix->drawPixelRGB888(mirrorBaseX + (panelWidth - 1 - x), offsetY + y, r, g, b);

                x++;
                if (x >= imageWidth) {
                    x = 0;
                    y++;
                    if (y >= imageHeight) return;
                }
            }
        } else {
            // Skip transparent pixels
            x += count;
            while (x >= imageWidth) {
                x -= imageWidth;
                y++;
                if (y >= imageHeight) return;
            }
        }
    }
}

void Hub75DMA::drawBitmapRLE(const uint8_t* rleBitmap, int imageWidth, int imageHeight, int offsetX, int offsetY, uint8_t r, uint8_t g, uint8_t b) {
    int i = 0;
    for (int row = 0; row < imageHeight; row++) {
        const int offsetYPlusRow = offsetY + row;
        int col = 0;

        while (col < imageWidth) {
            const uint8_t count = pgm_read_byte(rleBitmap + i++);
            const uint8_t value = pgm_read_byte(rleBitmap + i++);

            if (value > minimumPixelBrightness) {
                const int endCol = col + count;
                const int drawEnd = (endCol < imageWidth) ? endCol : imageWidth;
                for (int c = col; c < drawEnd; c++) {
                    matrix->drawPixelRGB888(offsetX + c, offsetYPlusRow, r, g, b);
                }
            }
            col += count;
        }
    }
}

void Hub75DMA::drawBitmapRLE(const uint8_t* rleBitmapL, const uint8_t* rleBitmapR, int imageWidth, int imageHeight, int offsetX, int offsetY) {
    const int mirrorBaseX = -offsetX + panelWidth;  // Consistent with drawBitmap

    // Draw left bitmap
    int iL = 0, xL = 0, yL = 0;
    while (yL < imageHeight) {
        const uint8_t countL = pgm_read_byte(rleBitmapL + iL++);
        const uint8_t valueL = pgm_read_byte(rleBitmapL + iL++);

        if (valueL > minimumPixelBrightness) {
            uint8_t r, g, b;
            getColorMap(valueL, yL + offsetY, r, g, b);

            for (uint8_t c = 0; c < countL; c++) {
                matrix->drawPixelRGB888(offsetX + xL, offsetY + yL, r, g, b);

                xL++;
                if (xL >= imageWidth) {
                    xL = 0;
                    yL++;
                    if (yL >= imageHeight) break;
                }
            }
        } else {
            // Skip transparent pixels
            xL += countL;
            while (xL >= imageWidth) {
                xL -= imageWidth;
                yL++;
                if (yL >= imageHeight) break;
            }
        }
    }

    // Draw right bitmap
    int iR = 0, xR = 0, yR = 0;
    while (yR < imageHeight) {
        const uint8_t countR = pgm_read_byte(rleBitmapR + iR++);
        const uint8_t valueR = pgm_read_byte(rleBitmapR + iR++);

        if (valueR > minimumPixelBrightness) {
            uint8_t r, g, b;
            getColorMap(valueR, yR + offsetY, r, g, b);

            for (uint8_t c = 0; c < countR; c++) {
                matrix->drawPixelRGB888(mirrorBaseX + (panelWidth - 1 - xR), offsetY + yR, r, g, b);

                xR++;
                if (xR >= imageWidth) {
                    xR = 0;
                    yR++;
                    if (yR >= imageHeight) break;
                }
            }
        } else {
            // Skip transparent pixels
            xR += countR;
            while (xR >= imageWidth) {
                xR -= imageWidth;
                yR++;
                if (yR >= imageHeight) break;
            }
        }
    }
}
