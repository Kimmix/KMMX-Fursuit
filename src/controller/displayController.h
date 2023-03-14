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

#define PANEL_RES_X 64
#define PANEL_RES_Y 32
#define PANELS_NUMBER 2
#define SCREEN_WIDTH PANEL_RES_X* PANELS_NUMBER
#define SCREEN_HEIGHT PANEL_RES_Y

class DisplayController {
private:
	MatrixPanel_I2S_DMA* matrix;
	const int panelWidth, panelHeight;

public:
	DisplayController(): panelWidth(PANEL_RES_X), panelHeight(PANEL_RES_Y) {
		HUB75_I2S_CFG mxconfig(panelWidth, panelHeight, PANELS_NUMBER);
		mxconfig.double_buff = true; // Turn of double buffer
		// mxconfig.clkphase = true;
		matrix = new MatrixPanel_I2S_DMA(mxconfig);
		matrix->clearScreen();
		delay(200);
		if (!matrix->begin())
			Serial.println("****** I2S memory allocation failed ***********");
	}

	void clearScreen() {
		matrix->clearScreen();
	}

	void render() {
		matrix->flipDMABuffer();
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
	void drawBitmap(const uint8_t* bitmap, int imageWidth, int imageHeight, int offsetX, int offsetY) {
		for (int i = 0; i < imageHeight; i++) {
			for (int j = 0, j2 = panelWidth - 1; j < imageWidth; j++, j2--) {
				uint8_t r, g, b;
				uint8_t pixel = pgm_read_byte(bitmap + i * imageWidth + j); // read the bytes from program memory
				getColorMap(pixel, i + offsetY, r, g, b);
				matrix->drawPixelRGB888(offsetX + j, offsetY + i, r, g, b);
				matrix->drawPixelRGB888(-offsetX + panelWidth + j2, offsetY + i, r, g, b);
			}
		}
	}

	void drawFullscreen(const uint8_t* bitmap) {
		drawBitmap(bitmap, panelWidth, panelHeight, 0, 0);
	}

	void drawEye(const uint8_t* bitmap) {
		drawBitmap(bitmap, 32, 18, 6, 0);
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