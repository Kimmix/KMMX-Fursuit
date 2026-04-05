#include "Hub75DMA.h"

Hub75DMA::Hub75DMA() {
    HUB75_I2S_CFG::i2s_pins _pins = {R1, G1, BL1, R2, G2, BL2, CH_A, CH_B, CH_C, CH_D, CH_E, LAT, OE, CLK};
    HUB75_I2S_CFG mxconfig(panelWidth, panelHeight, panelsNumber, _pins);
    mxconfig.min_refresh_rate = minRefreshRate;
    mxconfig.double_buff = doubleBuffer;
    matrix = new MatrixPanel_I2S_DMA(mxconfig);
    if (!matrix->begin())
        Serial.println(F("****** I2S memory allocation failed ***********"));
    matrix->setBrightness8(panelBrightness);
    matrix->clearScreen();
}

int Hub75DMA::getRefreshRate() {
    return matrix->calculated_refresh_rate;
}

void Hub75DMA::flipDMABuffer() {
    matrix->flipDMABuffer();
}

void Hub75DMA::clearScreen() {
    matrix->clearScreen();
}

int Hub75DMA::getBrightnessValue() {
    return panelBrightness;
}

int Hub75DMA::getResX() {
    return panelResX;
}

uint16_t Hub75DMA::color565(uint8_t r, uint8_t g, uint8_t b) {
    return matrix->color565(r, g, b);
}

void Hub75DMA::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    matrix->drawRect(x, y, w, h, color);
}

void Hub75DMA::drawPixel(int16_t x, int16_t y, uint16_t color) {
    matrix->drawPixel(x, y, color);
}

void Hub75DMA::setTextColor(uint16_t color) {
    matrix->setTextColor(color);
}

void Hub75DMA::setCursor(int16_t x, int16_t y) {
    matrix->setCursor(x, y);
}

void Hub75DMA::print(char str[]) {
    matrix->print(str);
}

void Hub75DMA::print(int n) {
    matrix->print(n);
}

void Hub75DMA::getTextBounds(const String& str, int16_t x, int16_t y, int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h) {
    matrix->getTextBounds(str, x, y, x1, y1, w, h);
}

void Hub75DMA::setBrightnessValue(uint8_t value) {
    panelBrightness = constrain(value, 0, 255);
    matrix->setBrightness8(panelBrightness);
}

void Hub75DMA::setColorMode(uint8_t mode) {
    // Valid modes: 0 = gradient (customizable), 1 = spiral, 2 = plasma, 3 = radial pulse, 4 = dual spiral, 5 = dual circle
    if (mode <= 5) {
        colorMode = mode;
    }
}

uint8_t Hub75DMA::getColorMode() const {
    return colorMode;
}

void Hub75DMA::setGradientColors(uint8_t topR, uint8_t topG, uint8_t topB, uint8_t bottomR, uint8_t bottomG, uint8_t bottomB) {
    colorEffects.setGradientColors(topR, topG, topB, bottomR, bottomG, bottomB);
}

void Hub75DMA::getGradientTopColor(uint8_t& r, uint8_t& g, uint8_t& b) const {
    colorEffects.getGradientTopColor(r, g, b);
}

void Hub75DMA::getGradientBottomColor(uint8_t& r, uint8_t& g, uint8_t& b) const {
    colorEffects.getGradientBottomColor(r, g, b);
}

void Hub75DMA::setDualSpiralColor(uint8_t spiralR, uint8_t spiralG, uint8_t spiralB) {
    colorEffects.setDualSpiralColor(spiralR, spiralG, spiralB);
}

void Hub75DMA::getDualSpiralColor(uint8_t& r, uint8_t& g, uint8_t& b) const {
    colorEffects.getDualSpiralColor(r, g, b);
}

void Hub75DMA::setDualCircleColor(uint8_t circleR, uint8_t circleG, uint8_t circleB) {
    colorEffects.setDualCircleColor(circleR, circleG, circleB);
}

void Hub75DMA::getDualCircleColor(uint8_t& r, uint8_t& g, uint8_t& b) const {
    colorEffects.getDualCircleColor(r, g, b);
}

void Hub75DMA::setEffectThickness(uint8_t thickness) {
    colorEffects.setEffectThickness(thickness);
}

uint8_t Hub75DMA::getEffectThickness() const {
    return colorEffects.getEffectThickness();
}

void Hub75DMA::setEffectSpeed(uint8_t speed) {
    colorEffects.setEffectSpeed(speed);
}

uint8_t Hub75DMA::getEffectSpeed() const {
    return colorEffects.getEffectSpeed();
}

void Hub75DMA::getColorMap(const uint8_t lightness, const int row, const int col, uint8_t& r, uint8_t& g, uint8_t& b) {
    colorEffects.getColor(colorMode, lightness, row, col, r, g, b);
}

void Hub75DMA::colorSpiral(const uint8_t brightness, const int startX, const int startY, const int row, const int col, uint8_t& r, uint8_t& g, uint8_t& b) {
    const uint8_t color1R = 255;
    const uint8_t color1G = 0;
    const uint8_t color1B = 0;

    const uint8_t color2R = 0;
    const uint8_t color2G = 0;
    const uint8_t color2B = 255;

    int dx = col - startX;
    int dy = row - startY;
    int distanceSquared = dx * dx + dy * dy;

    if (distanceSquared % 4 == 0) {
        r = color1R;
        g = color1G;
        b = color1B;
    } else {
        r = color2R;
        g = color2G;
        b = color2B;
    }

    r = r * brightness / 255;
    g = g * brightness / 255;
    b = b * brightness / 255;
}

void Hub75DMA::getBlackWhiteWave(const uint8_t brightness, const int row, const int col, uint8_t& r, uint8_t& g, uint8_t& b) {
    float wave = (row + col) * 0.1f + millis() * 0.008f;
    float gray = sinf(wave) * 127.0f + 128.0f;
    r = g = b = static_cast<uint8_t>(gray);

    float scale = brightness / 255.0f;
    gray = gray * scale;

    r = g = b = constrain(static_cast<uint8_t>(gray), 0, 255);
}

void Hub75DMA::getColorWave(const uint8_t brightness, const int row, const int col, uint8_t& r, uint8_t& g, uint8_t& b) {
    float wave = (row + col) * 0.4f + millis() * 0.01f;
    float color1 = sinf(wave) * 127.0f + 128.0f;
    float color2 = sinf(wave + PI) * 127.0f + 128.0f;

    float scale = brightness / 255.0f;
    color1 *= scale;
    color2 *= scale;

    r = constrain(static_cast<uint8_t>(color2), 0, 255);
    g = constrain(static_cast<uint8_t>(color1), 0, 255);
    b = constrain(static_cast<uint8_t>(color1 + color2), 0, 255);
}
