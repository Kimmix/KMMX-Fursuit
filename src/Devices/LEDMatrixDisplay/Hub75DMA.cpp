#include "Hub75DMA.h"

Hub75DMA::Hub75DMA() {
    HUB75_I2S_CFG::i2s_pins _pins = {R1, G1, BL1, R2, G2, BL2, CH_A, CH_B, CH_C, CH_D, CH_E, LAT, OE, CLK};
    HUB75_I2S_CFG mxconfig(panelWidth, panelHeight, PANELS_NUMBER, _pins);
    mxconfig.driver = HUB75_I2S_CFG::FM6124;
    mxconfig.clkphase = false;
    mxconfig.min_refresh_rate = 143;
    mxconfig.double_buff = true;
    matrix = new MatrixPanel_I2S_DMA(mxconfig);
    if (!matrix->begin())
        Serial.println(F("****** I2S memory allocation failed ***********"));
    matrix->setBrightness8(panelBrightness);
    matrix->clearScreen();
}

void Hub75DMA::render() {
    matrix->flipDMABuffer();
}

void Hub75DMA::clearScreen() {
    matrix->clearScreen();
}

int Hub75DMA::getBrightnessValue() {
    return panelBrightness;
}

int Hub75DMA::getResX() {
    return PANEL_RES_X;
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
    if (value < 0) {
        value = 0;
    } else if (value > 255) {
        value = 255;
    }
    panelBrightness = value;
    matrix->setBrightness8(panelBrightness);
}

void Hub75DMA::getColorMap(const uint8_t lightness, const int row, uint8_t& r, uint8_t& g, uint8_t& b) {
    const int index = row * 3;
    if (row >= 0 && index < sizeof(accColor)) {
        uint16_t factor = lightness << 8;
        r = (factor * accColor[index]) >> 16;
        g = (factor * accColor[index + 1]) >> 16;
        b = (factor * accColor[index + 2]) >> 16;
    } else {
        r = g = b = 0;
    }
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
    double wave = (row + col) * 0.1 + millis() * 0.008;
    double gray = sin(wave) * 127 + 128;
    r = g = b = static_cast<uint8_t>(gray);

    double scale = brightness / 255.0;
    gray = gray * scale;

    r = g = b = constrain(static_cast<uint8_t>(gray), 0, 255);
}

void Hub75DMA::getColorWave(const uint8_t brightness, const int row, const int col, uint8_t& r, uint8_t& g, uint8_t& b) {
    double wave = (row + col) * 0.4 + millis() * 0.01;
    double color1 = sin(wave) * 127 + 128;
    double color2 = sin(wave + PI) * 127 + 128;

    double scale = brightness / 255.0;
    color1 *= scale;
    color2 *= scale;

    r = constrain(static_cast<uint8_t>(color2), 0, 255);
    g = constrain(static_cast<uint8_t>(color1), 0, 255);
    b = constrain(static_cast<uint8_t>(color1 + color2), 0, 255);
}
