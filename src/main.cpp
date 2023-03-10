#include "main.h"
#include <Arduino.h>
#include "BLE.h"
#include "Icons.h"
#include "draw.h"

BLEControl bleController = BLEControl();

// *** Led Matrix ***
MatrixPanel_I2S_DMA* matrix = nullptr;
VirtualMatrixPanel_FastLED_Pixel_Buffer* FastLED_Pixel_Buff = nullptr;

void drawHeart(int offsetX, int offsetY) {
	int imageWidth = 5, imageHeight = 5;
	// int offsetX = 0, offsetY = 0;
	int i, j, j2;
	for (i = 0; i < imageHeight; i++) {
		for (j = 0, j2 = 63; j < imageWidth; j++) {
			if (heartS[i * imageWidth + j] > 0) {
				FastLED_Pixel_Buff->drawPixel(offsetX + j, offsetY + i, (heartS[i * imageWidth + j] * 255) / 255, 0, 0);
				// matrix->drawPixelRGB888(-offsetX + PANEL_WIDTH + j2, offsetY + i,
				//                         (heartS[i * imageWidth + j] * 255) / 255, 0,
				//                         0);
				j2--;
			}
		}
	}
}

struct Heart {
	float xpos, ypos;
	float velocityx, velocityy;
};
const int numHeart = 5;
Heart Hearts[numHeart];
volatile unsigned long heartSpeed = 0;
void flyingHeart() {
	if (millis() - heartSpeed >= 25) {
		FastLED_Pixel_Buff->dimAll(200);
		for (int i = 0; i < numHeart; i++) {
			// Draw heart and then calculate
			drawHeart(Hearts[i].xpos, Hearts[i].ypos);
			if (8 + Hearts[i].xpos >= FastLED_Pixel_Buff->width()) {
				Hearts[i].velocityx *= -1;
			}
			else if (Hearts[i].xpos <= 0) {
				Hearts[i].velocityx = abs(Hearts[i].velocityx);
			}
			if (6 + Hearts[i].ypos >= FastLED_Pixel_Buff->height()) {
				Hearts[i].velocityy *= -1;
			}
			else if (Hearts[i].ypos <= 0) {
				Hearts[i].velocityy = abs(Hearts[i].velocityy);
			}
			Hearts[i].xpos += Hearts[i].velocityx;
			Hearts[i].ypos += Hearts[i].velocityy;
			heartSpeed = millis();
		}
		FastLED_Pixel_Buff->show();
	}
}

void setup() {
	Serial.begin(BAUD_RATE);
	pinMode(IR_PIN, INPUT);
	randomSeed(analogRead(0));
	bleController.Initialize();

	// ------ Setup P3 LED Matrix Pannel ------
	HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANELS_NUMBER);
	// mxconfig.driver = HUB75_I2S_CFG::ICN2038S;
	// mxconfig.double_buff = true; // Turn of double buffer
	mxconfig.clkphase = false;
	matrix = new MatrixPanel_I2S_DMA(mxconfig);
	matrix->setBrightness8(125);  // 0-255
	matrix->clearScreen();
	if (not matrix->begin())
		Serial.println("****** I2S memory allocation failed ***********");
	FastLED_Pixel_Buff = new VirtualMatrixPanel_FastLED_Pixel_Buffer(
		(*matrix), 1, PANELS_NUMBER, PANEL_WIDTH, PANEL_HEIGHT, true, false);
	if (not FastLED_Pixel_Buff->allocateMemory())
		Serial.println(
			"****** Unable to find enough memory for the FastLED pixel "
			"buffer! ***********");
	// drawGSBitmap(face_gs);
	drawEye(eyeDefault);
	drawNose(noseDefault);
	drawMouth(mouthDefault);
	drawColorTest();

	for (int i = 0; i < numHeart; i++) {
		// Hearts[i].xpos = random(0, matrix->width() - 8);
		// Hearts[i].ypos = random(0, matrix->height() - 6);
		Hearts[i].xpos = 0;
		Hearts[i].ypos = random(0, matrix->height() - 5);
		Hearts[i].velocityx = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		Hearts[i].velocityy = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	}
}

bool isBoop;

void loop() {
	bleController.start();
	// BLE.poll();  // Start BLE
	// matrix->flipDMABuffer();
	// matrix->clearScreen();
	// drawEye(eyeDefault);
	// drawNose(noseDefault);
	// drawMouth(mouthDefault);
	// flyingHeart();
	isBoop = !digitalRead(IR_PIN);
	if (isBoop) {
		// flyingHeart();
		boop(isBoop);
	}
	else {
		blink();
	}
	// oFace();
	// Serial.println("Fill screen: RED");
	// matrix->fillScreenRGB888(255, 0, 0);
	// delay(PATTERN_DELAY);
	// draw_eye(0, 0, myEye);
}