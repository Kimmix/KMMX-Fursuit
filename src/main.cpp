#include "main.h"
#include <Arduino.h>
#include "controller/BLEController.h"
#include "controller/displayController.h"
#include "Icons.h"
#include "draw.h"

BLEController bleController = BLEController();
DisplayController displayController = DisplayController();


void setup() {
	Serial.begin(BAUD_RATE);
	pinMode(IR_PIN, INPUT);
	// bleController.Initialize();
	displayController.init();
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
	// delay(25);
}