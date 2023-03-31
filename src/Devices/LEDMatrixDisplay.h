#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

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

#define PANEL_RES_X 64
#define PANEL_RES_Y 32
#define PANELS_NUMBER 2
#define SCREEN_WIDTH PANEL_RES_X* PANELS_NUMBER
#define SCREEN_HEIGHT PANEL_RES_Y

class DisplayController {
private:
	MatrixPanel_I2S_DMA* matrix;
	const int panelWidth, panelHeight;
	uint8_t panelBrightness = 64;
	int eyeWidth = 32, eyeHeight = 18;

public:
	DisplayController(): panelWidth(PANEL_RES_X), panelHeight(PANEL_RES_Y) {
		HUB75_I2S_CFG mxconfig(panelWidth, panelHeight, PANELS_NUMBER);
		// mxconfig.double_buff = true; // Turn of double buffer
		// mxconfig.clkphase = true;
		matrix = new MatrixPanel_I2S_DMA(mxconfig);
		if (!matrix->begin())
			Serial.println("****** I2S memory allocation failed ***********");
		matrix->setBrightness8(panelBrightness);
		matrix->clearScreen();
	}

	void render() {
		matrix->flipDMABuffer();
	}

	void clearScreen() {
		matrix->clearScreen();
	}

	int getBrightnessValue() {
		return panelBrightness;
	}

	void setBrightnessValue(uint8_t value) {
		// Ensure that the brightness value is between 0 and 255
		if (value < 0) {
			value = 0;
		}
		else if (value > 255) {
			value = 255;
		}
		panelBrightness = value;
		matrix->setBrightness8(panelBrightness);
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
			uint16_t factor = lightness << 8; // multiply by 256 (i.e., 2^8) using bit shift
			r = (factor * accColor[index]) >> 16; // divide by 65536 (i.e., 2^16) using bit shift
			g = (factor * accColor[index + 1]) >> 16;
			b = (factor * accColor[index + 2]) >> 16;
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
	void drawBitmap(const uint8_t* bitmap, int imageWidth, int imageHeight, int offsetX, int offsetY, bool drawBlack = true) {
		uint8_t r, g, b;
		for (int i = 0; i < imageHeight; i++) {
			for (int j = 0, j2 = panelWidth - 1; j < imageWidth; j++, j2--) {
				uint8_t pixel = pgm_read_byte(bitmap + i * imageWidth + j); // read the bytes from program memory
				getColorMap(pixel, i + offsetY, r, g, b);
				if (drawBlack || (r != 0 || g != 0 || b != 0)) {
					matrix->drawPixelRGB888(offsetX + j, offsetY + i, r, g, b);
					matrix->drawPixelRGB888(-offsetX + panelWidth + j2, offsetY + i, r, g, b);
				}
			}
		}
	}

	void drawFullscreen(const uint8_t* bitmap) {
		drawBitmap(bitmap, panelWidth, panelHeight, 0, 0);
	}

	void drawEye(const uint8_t* bitmap) {
		drawBitmap(bitmap, eyeWidth, eyeHeight, 6, 0);
	}

	void drawEyePupil(const uint8_t* bitmap, int x, int y) {
		drawBitmap(bitmap, 6, 6, 14 + x, 5 + y, false);
	}

	void drawNose(const uint8_t* bitmap) {
		drawBitmap(bitmap, 10, 6, 54, 6);
	}

	void drawMouth(const uint8_t* bitmap) {
		drawBitmap(bitmap, 50, 14, 14, 18);
	}

	void drawColorTest() {
		uint8_t r, g, b;
		for (int i = 0; i < 64; i++) {
			getColorMap(255, i, r, g, b);
			matrix->drawPixelRGB888(0, i, r, g, b);
			matrix->drawPixelRGB888(SCREEN_WIDTH - 1, i, r, g, b);
		}
	}
};