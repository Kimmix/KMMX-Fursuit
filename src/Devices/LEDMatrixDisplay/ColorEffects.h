#pragma once

#include <Arduino.h>

/**
 * @brief Color effects for LED matrix display
 *
 * This class provides various color effects and color modes for the LED matrix.
 * Each mode can generate different visual effects based on position and time.
 */
class ColorEffects {
   public:
    /**
     * @brief Constructor
     * @param width Panel width in pixels
     * @param height Panel height in pixels
     */
    ColorEffects(uint8_t width, uint8_t height);

    /**
     * @brief Get color based on the current color mode
     * @param mode Color mode:
     *        0 = Gradient (customizable via setGradientColors)
     *        1 = Spiral/Vortex (VRChat hypno style)
     *        2 = Plasma Effect (trippy interference patterns)
     *        3 = Radial Pulse (breathing effect from center)
     * @param lightness Lightness/brightness value (0-255)
     * @param row Row index (Y coordinate)
     * @param col Column index (X coordinate)
     * @param r Reference to red component (output)
     * @param g Reference to green component (output)
     * @param b Reference to blue component (output)
     */
    void getColor(uint8_t mode, uint8_t lightness, int row, int col, uint8_t& r, uint8_t& g, uint8_t& b);

    /**
     * @brief Set gradient colors for Mode 0
     * Use this to customize the gradient or reset to defaults
     * Default: Top RGB(255,163,147) Bottom RGB(255,43,91)
     * @param topR Top color red component (0-255)
     * @param topG Top color green component (0-255)
     * @param topB Top color blue component (0-255)
     * @param bottomR Bottom color red component (0-255)
     * @param bottomG Bottom color green component (0-255)
     * @param bottomB Bottom color blue component (0-255)
     */
    void setGradientColors(uint8_t topR, uint8_t topG, uint8_t topB, uint8_t bottomR, uint8_t bottomG, uint8_t bottomB);

    /**
     * @brief Get the top gradient color
     */
    void getGradientTopColor(uint8_t& r, uint8_t& g, uint8_t& b) const;

    /**
     * @brief Get the bottom gradient color
     */
    void getGradientBottomColor(uint8_t& r, uint8_t& g, uint8_t& b) const;

   private:
    uint8_t panelWidth;
    uint8_t panelHeight;

    // Gradient colors for Mode 0 (default KMMX colors, customizable)
    uint8_t gradientTopR = 255;
    uint8_t gradientTopG = 163;
    uint8_t gradientTopB = 147;
    uint8_t gradientBottomR = 255;
    uint8_t gradientBottomG = 43;
    uint8_t gradientBottomB = 91;

    /**
     * @brief Mode 0: Customizable gradient
     */
    void modeGradient(uint8_t lightness, int row, uint8_t& r, uint8_t& g, uint8_t& b);

    /**
     * @brief Mode 1: Spiral/Vortex effect with rainbow HSV cycling
     */
    void modeSpiralVortex(uint8_t lightness, int row, int col, uint8_t& r, uint8_t& g, uint8_t& b);

    /**
     * @brief Mode 2: Plasma effect with multiple sine wave interference
     */
    void modePlasmaEffect(uint8_t lightness, int row, int col, uint8_t& r, uint8_t& g, uint8_t& b);

    /**
     * @brief Mode 3: Radial pulse/breathing effect from center
     */
    void modeRadialPulse(uint8_t lightness, int row, int col, uint8_t& r, uint8_t& g, uint8_t& b);

    /**
     * @brief Helper: Convert HSV to RGB
     * @param h Hue (0.0 - 6.0)
     * @param s Saturation (0.0 - 1.0)
     * @param v Value/brightness (0.0 - 1.0)
     * @param r Reference to red component (output)
     * @param g Reference to green component (output)
     * @param b Reference to blue component (output)
     */
    void hsvToRgb(float h, float s, float v, uint8_t& r, uint8_t& g, uint8_t& b);
};

