#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "FastLED_Pixel_Buffer.h"

// ---- P3 LED Matrix Panel ----
// HUB75E pinout
// R1 | G1
// B1 | GND
// R2 | G2
// B2 | E
//  A | B
//  C | D
// CLK| LAT
// OE | GND

// Default library pin configuration for the reference you can redefine only ones you need later on object creation
// #define R1 25
// #define G1 26
// #define BL1 27
// #define R2 14
// #define G2 12
// #define BL2 13
// #define CH_A 23
// #define CH_B 19
// #define CH_C 5
// #define CH_D 17
// #define CH_E -1 // assign to any available pin if using panels with 1/32 scan
// #define CLK 16
// #define LAT 4

// extern MatrixPanel_I2S_DMA* matrix;
// extern VirtualMatrixPanel_FastLED_Pixel_Buffer* FastLED_Pixel_Buff;

#define PANEL_WIDTH 64
#define PANEL_HEIGHT 32
#define PANELS_NUMBER 2
#define SCREEN_WIDTH PANEL_WIDTH* PANELS_NUMBER
#define SCREEN_HEIGHT PANEL_HEIGHT

// *** Led Matrix ***
MatrixPanel_I2S_DMA* matrix;
VirtualMatrixPanel_FastLED_Pixel_Buffer* FastLED_Pixel_Buff;

class DisplayController {
public:
	DisplayController(): kPanelWidth(PANEL_WIDTH), kPanelHeight(PANEL_HEIGHT) {}

	void init() {
		// ------ Setup P3 LED Matrix Pannel ------
		HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANELS_NUMBER);
		matrix = new MatrixPanel_I2S_DMA(mxconfig);
		matrix->clearScreen();
		delay(500);
		if (!matrix->begin())
			Serial.println("****** I2S memory allocation failed ***********");
		FastLED_Pixel_Buff = new VirtualMatrixPanel_FastLED_Pixel_Buffer((*matrix), 1, PANELS_NUMBER, PANEL_WIDTH, PANEL_HEIGHT, true, false);
		if (!FastLED_Pixel_Buff->allocateMemory())
			Serial.println("****** Unable to find enough memory for the FastLED pixel buffer! ***********");
	}

	/**
	 * @brief Returns the RGB value of a color in a gradient
	 * @param lightness Brightness of color (8-bit)
	 * @param row Current row of the LED matrix
	 * @param uint8_t r - RGB888 color
	 * @param uint8_t g - RGB888 color
	 * @param uint8_t b - RGB888 color
	 */
	void getColorMap(const uint8_t lightness, const int row, uint8_t& r, uint8_t& g, uint8_t& b) {
		const int index = row * 3;
		if (row >= 0 && index < sizeof(accColor)) { // add a check for valid range
			r = (lightness * accColor[index]) >> 8;
			g = (lightness * accColor[index + 1]) >> 8;
			b = (lightness * accColor[index + 2]) >> 8;
		}
		else {
			// handle the case where row is out of range
			r = g = b = 0;
		}
	}

	/**
	 * @brief Draws a bitmap onto the LED matrix
	 * @param bitmap Array of 8-bit values representing the image
	 * @param image_width Width of the image in pixels
	 * @param image_height Height of the image in pixels
	 * @param offset_x X offset of the image
	 * @param offset_y Y offset of the image
	 */
	void drawBitmap(const uint8_t bitmap[], int imageWidth, int imageHeight, int offsetX, int offsetY) {
		for (int i = 0; i < imageHeight; i++) {
			for (int j = 0, j2 = kPanelWidth - 1; j < imageWidth; j++, j2--) {
				uint8_t r, g, b;
				getColorMap(bitmap[i * imageWidth + j], i + offsetY, r, g, b);
				FastLED_Pixel_Buff->drawPixel(offsetX + j, offsetY + i, r, g, b);
				FastLED_Pixel_Buff->drawPixel(-offsetX + kPanelWidth + j2, offsetY + i, r, g, b);
			}
		}
	}

	void drawGSBitmap(const uint8_t bitmap[]) {
		drawBitmap(bitmap, PANEL_WIDTH, PANEL_HEIGHT, 0, 0);
	}

	void drawEye(const uint8_t bitmap[]) {
		drawBitmap(bitmap, 32, 18, 6, 0);
	}

	void drawNose(const uint8_t bitmap[]) {
		drawBitmap(bitmap, 10, 6, 54, 6);
	}

	void drawMouth(const uint8_t bitmap[]) {
		drawBitmap(bitmap, 50, 14, 14, 18);
	}

	void drawColorTest() {
		uint8_t r, g, b;
		for (int i = 0; i < 64; i++) {
			getColorMap(255, i, r, g, b);
			FastLED_Pixel_Buff->drawPixel(0, i, r, g, b);
			FastLED_Pixel_Buff->drawPixel(SCREEN_WIDTH - 1, i, r, g, b);
		}
	}

private:
	const int kPanelWidth;
	const int kPanelHeight;
};