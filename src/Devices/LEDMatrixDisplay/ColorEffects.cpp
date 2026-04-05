#include "ColorEffects.h"
#include "Bitmaps/Bitmaps.h"
#include <math.h>

ColorEffects::ColorEffects(uint8_t width, uint8_t height)
    : panelWidth(width), panelHeight(height) {
}

void ColorEffects::getColor(uint8_t mode, uint8_t lightness, int row, int col, uint8_t& r, uint8_t& g, uint8_t& b) {
    switch (mode) {
        case 0:
            modeGradient(lightness, row, r, g, b);
            break;
        case 1:
            modeSpiralVortex(lightness, row, col, r, g, b);
            break;
        case 2:
            modePlasmaEffect(lightness, row, col, r, g, b);
            break;
        case 3:
            modeRadialPulse(lightness, row, col, r, g, b);
            break;
        default:
            // Fallback to gradient
            modeGradient(lightness, row, r, g, b);
            break;
    }
}

void ColorEffects::setGradientColors(uint8_t topR, uint8_t topG, uint8_t topB, uint8_t bottomR, uint8_t bottomG, uint8_t bottomB) {
    gradientTopR = topR;
    gradientTopG = topG;
    gradientTopB = topB;
    gradientBottomR = bottomR;
    gradientBottomG = bottomG;
    gradientBottomB = bottomB;
}

void ColorEffects::getGradientTopColor(uint8_t& r, uint8_t& g, uint8_t& b) const {
    r = gradientTopR;
    g = gradientTopG;
    b = gradientTopB;
}

void ColorEffects::getGradientBottomColor(uint8_t& r, uint8_t& g, uint8_t& b) const {
    r = gradientBottomR;
    g = gradientBottomG;
    b = gradientBottomB;
}

// ============================================================================
// Mode 0: Customizable Gradient
// ============================================================================
void ColorEffects::modeGradient(uint8_t lightness, int row, uint8_t& r, uint8_t& g, uint8_t& b) {
    // Interpolate between top and bottom colors based on row
    // row 0 = top color, row 31 = bottom color
    const int clampedRow = constrain(row, 0, 31);
    const uint16_t fraction = (clampedRow << 8) / 31;  // 0-256 range

    // Interpolate each color component
    const uint16_t invFraction = 256 - fraction;
    const uint8_t baseR = (gradientTopR * invFraction + gradientBottomR * fraction) >> 8;
    const uint8_t baseG = (gradientTopG * invFraction + gradientBottomG * fraction) >> 8;
    const uint8_t baseB = (gradientTopB * invFraction + gradientBottomB * fraction) >> 8;

    // Apply lightness/brightness
    const uint16_t factor = lightness << 8;
    r = (factor * baseR) >> 16;
    g = (factor * baseG) >> 16;
    b = (factor * baseB) >> 16;
}

// ============================================================================
// Mode 1: Spiral/Vortex Effect (VRChat Hypno Shader Inspired)
// ============================================================================
void ColorEffects::modeSpiralVortex(uint8_t lightness, int row, int col, uint8_t& r, uint8_t& g, uint8_t& b) {
    // Calculate position relative to center
    const float centerX = panelWidth / 2.0f;
    const float centerY = panelHeight / 2.0f;
    const float dx = col - centerX;
    const float dy = row - centerY;

    // Calculate angle and distance from center
    const float angle = atan2f(dy, dx);
    const float distance = sqrtf(dx * dx + dy * dy);

    // Create animated spiral effect
    const float time = millis() * 0.001f;  // Convert to seconds
    const float spiralSpeed = 2.0f;        // Rotation speed
    const float spiralTightness = 0.3f;    // How tight the spiral is

    // Combine angle and distance for spiral pattern
    float spiralValue = angle + distance * spiralTightness - time * spiralSpeed;

    // Normalize to 0-1 range using sine wave for smooth cycling
    float hue = fmodf(spiralValue / (2.0f * PI) + 1.0f, 1.0f);

    // Add radial pulse effect
    float pulse = sinf(distance * 0.2f - time * 3.0f) * 0.5f + 0.5f;

    // Convert HSV to RGB (hue cycling rainbow effect)
    // Hue: 0-1, Saturation: 1.0, Value: based on pulse and lightness
    float h = hue * 6.0f;
    float saturation = 1.0f;
    float value = (pulse * 0.3f + 0.7f) * (lightness / 255.0f);

    hsvToRgb(h, saturation, value, r, g, b);
}

// ============================================================================
// Mode 2: Plasma Effect (Demoscene Classic)
// ============================================================================
void ColorEffects::modePlasmaEffect(uint8_t lightness, int row, int col, uint8_t& r, uint8_t& g, uint8_t& b) {
    const float time = millis() * 0.001f;

    // Normalize coordinates to center
    const float x = (col - panelWidth / 2.0f) / (panelWidth / 2.0f);
    const float y = (row - panelHeight / 2.0f) / (panelHeight / 2.0f);

    // Multiple sine waves creating interference patterns
    float plasma1 = sinf(x * 5.0f + time);
    float plasma2 = sinf(y * 5.0f + time * 1.3f);
    float plasma3 = sinf((x + y) * 4.0f + time * 0.8f);
    float plasma4 = sinf(sqrtf(x * x + y * y) * 6.0f - time * 2.0f);

    // Combine all plasma components
    float plasmaValue = (plasma1 + plasma2 + plasma3 + plasma4) / 4.0f;

    // Convert to 0-1 range
    plasmaValue = (plasmaValue + 1.0f) * 0.5f;

    // Map to rainbow hue
    float hue = plasmaValue * 6.0f;
    float saturation = 0.9f;  // Slightly less saturated for smoother look
    float value = (lightness / 255.0f) * 0.9f + 0.1f;  // Keep some minimum brightness

    hsvToRgb(hue, saturation, value, r, g, b);
}

// ============================================================================
// Mode 3: Radial Pulse (Breathing Effect)
// ============================================================================
void ColorEffects::modeRadialPulse(uint8_t lightness, int row, int col, uint8_t& r, uint8_t& g, uint8_t& b) {
    const float time = millis() * 0.001f;

    // Calculate distance from center
    const float centerX = panelWidth / 2.0f;
    const float centerY = panelHeight / 2.0f;
    const float dx = col - centerX;
    const float dy = row - centerY;
    const float distance = sqrtf(dx * dx + dy * dy);

    // Maximum possible distance (corner to center)
    const float maxDist = sqrtf(centerX * centerX + centerY * centerY);
    const float normalizedDist = distance / maxDist;  // 0 at center, 1 at corners

    // Create breathing pulse from center
    const float breathSpeed = 1.5f;
    const float breathOffset = sinf(time * breathSpeed) * 0.5f + 0.5f;  // 0-1

    // Create expanding rings
    const float ringSpeed = 2.0f;
    const float ringFrequency = 5.0f;
    const float rings = sinf(normalizedDist * ringFrequency - time * ringSpeed) * 0.5f + 0.5f;

    // Combine breath and rings
    const float pulse = (breathOffset * 0.6f + rings * 0.4f);

    // Color cycling - slower than spiral
    const float hue = fmodf(time * 0.5f + normalizedDist * 2.0f, 1.0f) * 6.0f;
    const float saturation = 0.85f;
    const float value = pulse * (lightness / 255.0f);

    hsvToRgb(hue, saturation, value, r, g, b);
}

// ============================================================================
// Helper Functions
// ============================================================================
void ColorEffects::hsvToRgb(float h, float s, float v, uint8_t& r, uint8_t& g, uint8_t& b) {
    int i = (int)h;
    float f = h - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    float rf, gf, bf;
    switch (i % 6) {
        case 0: rf = v; gf = t; bf = p; break;
        case 1: rf = q; gf = v; bf = p; break;
        case 2: rf = p; gf = v; bf = t; break;
        case 3: rf = p; gf = q; bf = v; break;
        case 4: rf = t; gf = p; bf = v; break;
        default: rf = v; gf = p; bf = q; break;
    }

    r = constrain((uint8_t)(rf * 255.0f), 0, 255);
    g = constrain((uint8_t)(gf * 255.0f), 0, 255);
    b = constrain((uint8_t)(bf * 255.0f), 0, 255);
}

