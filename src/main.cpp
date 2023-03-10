#include "main.h"
#include <Arduino.h>
#include "BLE.h"
#include "Icons.h"
#include "draw.h"

BLEControl bleController = BLEControl();

// *** Led Matrix ***
MatrixPanel_I2S_DMA* matrix = nullptr;
VirtualMatrixPanel_FastLED_Pixel_Buffer* FastLED_Pixel_Buff = nullptr;


void setup() {
	Serial.begin(BAUD_RATE);
	pinMode(IR_PIN, INPUT);
	// bleController.Initialize();

	// ------ Setup P3 LED Matrix Pannel ------
	HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANELS_NUMBER);
	// mxconfig.clkphase = false;
	matrix = new MatrixPanel_I2S_DMA(mxconfig);
	matrix->setBrightness8(96);  // 0-255
	matrix->clearScreen();
	delay(500);
	if (not matrix->begin())
		Serial.println("****** I2S memory allocation failed ***********");
	FastLED_Pixel_Buff = new VirtualMatrixPanel_FastLED_Pixel_Buffer((*matrix), 1, PANELS_NUMBER, PANEL_WIDTH, PANEL_HEIGHT, true, false);
	if (not FastLED_Pixel_Buff->allocateMemory())
		Serial.println("****** Unable to find enough memory for the FastLED pixel buffer! ***********");
}

bool isBoop, isOverrideEye = false;

void loop() {
	// bleController.start();
	FastLED_Pixel_Buff->dimAll(200);
	drawColorTest();
	if (!isOverrideEye)
		drawEye(eyeDefault);
	drawNose(noseDefault);
	drawMouth(mouthDefault);
	// flyingHeart();
	isBoop = !digitalRead(IR_PIN);
	if (isBoop) {
		// flyingHeart();
		boop();
	}
	else {
		blink(isOverrideEye);
	}
	FastLED_Pixel_Buff->show();
	Serial.println(isOverrideEye);
	// delay(25);
}