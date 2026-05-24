#include "Hub75DMA.h"
#include "KMMXController/MotionDetectionConfig.h"

Hub75DMA::Hub75DMA() {
    HUB75_I2S_CFG::i2s_pins _pins = {R1, G1, BL1, R2, G2, BL2, CH_A, CH_B, CH_C, CH_D, CH_E, LAT, OE, CLK};
    HUB75_I2S_CFG mxconfig(panelWidth, panelHeight, panelsNumber, _pins);
    mxconfig.min_refresh_rate = minRefreshRate;
    mxconfig.double_buff = doubleBuffer;
    matrix = new MatrixPanel_I2S_DMA(mxconfig);
    matrix->begin();  // Initialize matrix
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

void Hub75DMA::updateColorEffectsFrame() {
    colorEffects.updateFrame();
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

void Hub75DMA::setEffectDirectionInverted(uint8_t inverted) {
    colorEffects.setEffectDirectionInverted(inverted);
}

uint8_t Hub75DMA::getEffectDirectionInverted() const {
    return colorEffects.getEffectDirectionInverted();
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

void Hub75DMA::triggerGlitch(int intensity) {
    glitchState.active = true;
    glitchState.startTime = millis();
    glitchState.intensity = constrain(intensity, 0, 100);

    // Calculate duration based on intensity using exponential curve
    // Formula: duration = minDuration + (maxDuration - minDuration) * (intensity/100)^2
    // This makes higher intensities last significantly longer
    float normalizedIntensity = glitchState.intensity / 100.0f;
    float intensitySquared = normalizedIntensity * normalizedIntensity;
    glitchState.duration = tapGlitchMinDuration +
                          (unsigned long)(intensitySquared * (tapGlitchMaxDuration - tapGlitchMinDuration));

    glitchState.lastUpdate = millis();
    glitchState.glitchRow = esp_random() % panelHeight;

    // Initial glitch shift using exponential scaling
    // Formula: shift = baseShift * (intensity/100)^2
    float shiftScale = intensitySquared * intensitySquared;  // ^4 for even more dramatic scaling
    int maxShift = (int)(20 * shiftScale);
    glitchState.glitchShift = (esp_random() % (maxShift * 2 + 1)) - maxShift;
    glitchState.cachedRandomShift = 0;

    // Chance for full-screen glitch or localized glitch
    if ((esp_random() % 100) < tapGlitchFullScreenChance) {
        glitchState.cachedProximity = 999;  // Full-screen glitch
    } else {
        glitchState.cachedProximity = (esp_random() % 3) + 1;  // Localized (1-3 rows)
    }
}

void Hub75DMA::updateGlitch() {
    if (!glitchState.active) {
        return;
    }

    unsigned long currentTime = millis();

    // Check if glitch duration has expired
    if (currentTime - glitchState.startTime >= glitchState.duration) {
        glitchState.active = false;
        glitchState.glitchRow = -1;
        glitchState.glitchShift = 0;
        glitchState.cachedRandomShift = 0;
        glitchState.cachedProximity = 2;  // Reset to default
        return;
    }

    // Update glitch parameters periodically during the effect
    if (currentTime - glitchState.lastUpdate >= tapGlitchUpdateInterval) {
        glitchState.lastUpdate = currentTime;
        glitchState.glitchRow = esp_random() % panelHeight;

        // Use exponential scaling for glitch shift
        // Formula: shift = baseShift * (intensity/100)^4
        // This creates dramatically more intense shifts at higher values
        float normalizedIntensity = glitchState.intensity / 100.0f;
        float intensityQuad = normalizedIntensity * normalizedIntensity * normalizedIntensity * normalizedIntensity;
        int maxShift = (int)(15 * intensityQuad * 3);  // Scale up to 45 pixels at max intensity
        glitchState.glitchShift = (esp_random() % (maxShift * 2 + 1)) - maxShift;

        // Chance for full-screen glitch or localized glitch
        if ((esp_random() % 100) < tapGlitchFullScreenChance) {
            glitchState.cachedProximity = 999;  // Full-screen glitch
        } else {
            glitchState.cachedProximity = (esp_random() % 3) + 1;  // Localized (1-3 rows)
        }

        // Calculate random shift once per update interval (not every frame at 200fps)
        // Use exponential scaling for random shift chance and magnitude
        int randomShiftChance = (int)(intensityQuad * 100);  // Exponential chance increase
        if ((esp_random() % 100) < randomShiftChance) {
            int maxRandomShift = (int)(5 * intensityQuad * 2);  // Scale up to 10 pixels at max intensity
            glitchState.cachedRandomShift = (esp_random() % (maxRandomShift * 2 + 1)) - maxRandomShift;
        } else {
            glitchState.cachedRandomShift = 0;
        }
    }
}

bool Hub75DMA::isGlitchActive() const {
    return glitchState.active;
}
