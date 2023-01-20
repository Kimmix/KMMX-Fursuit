#include "main.h"
#include "Icons.h"

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

void drawHeart() {
  int imageWidth = 8, imageHeight = 6;
  int i, j, j2;
  for (i = 0; i < imageHeight; i++) {
    for (j = 0, j2 = 63; j < imageWidth; j++) {
      if (heart[i * imageWidth + j] > 0) {
        matrix->drawPixelRGB888(j, i, (heart[i * imageWidth + j] * 255) / 255,
                                0, 0);
        matrix->drawPixelRGB888(PANEL_WIDTH + j2, i,
                                (heart[i * imageWidth + j] * 255) / 255, 0, 0);
        j2--;
      }
    }
  }
}

volatile unsigned long blinkTime = 0, blinkSpeed = 0, blinkSpeedInterval = 25, blinkInterval = 1000;
int Step = 0, blinkAnimationStep = 0;
const uint8_t* blinkAnimation[8] = {
    eyeDefault, eyeBlink1, eyeBlink2, eyeBlink3,
    eyeBlink4,  eyeBlink5, eyeBlink6, eyeBlink7,
};
void blink() {
  if (millis() - blinkTime >= blinkInterval) {
    if (millis() - blinkSpeed >= blinkSpeedInterval) {
      if (Step < 8) {
        blinkSpeedInterval = 30;
        drawEye(blinkAnimation[blinkAnimationStep]);
        blinkAnimationStep++;
        Step++;
      } else if (Step >= 8 && Step < 14) {
        blinkSpeedInterval = 15;
        blinkAnimationStep--;
        drawEye(blinkAnimation[blinkAnimationStep]);
        Step++;
      } else if (Step >= 14) {
        blinkSpeedInterval = 80;
        blinkAnimationStep--;
        drawEye(blinkAnimation[blinkAnimationStep]);
        Step++;
      }
      if (Step == 16) {
        blinkAnimationStep = 0;
        Step = 0;
        blinkInterval = (1000 * random(4, 12));
        blinkTime = millis();
      }
      blinkSpeed = millis();
    }
  }
}

volatile unsigned long boopSpeed = 0;
const uint8_t* boopAnimation[2] = {eyeV1, eyeV2};
int boopAnimationFrame = 0;
void boop(bool isBoop) {
  bool isConfused = !!random(2);
  while (isBoop) {
    if (millis() - boopSpeed >= 250) {
      if (isConfused) {
        oFace();
      } else {
        boopAnimationFrame ^= 1;
        drawEye(boopAnimation[boopAnimationFrame]);
        boopSpeed = millis();
      }
    }
    isBoop = !digitalRead(IR_PIN);
  }
  blinkTime = millis();
  drawEye(eyeDefault);
}

volatile unsigned long oFaceSpeed = 0;
const uint8_t* oFaceAnimation[3] = {eyeO1, eyeO2, eyeO3};
int oFaceAnimationFrame = 0;
void oFace() {
  if (millis() - oFaceSpeed >= 250) {
    oFaceAnimationFrame = random(0, 2);
    drawEye(oFaceAnimation[oFaceAnimationFrame]);
    oFaceSpeed = millis();
  }
}