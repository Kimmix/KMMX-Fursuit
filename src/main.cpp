#include "main.h"
#include <Arduino.h>
#include "controller/BLEController.h"
#include "controller/displayController.h"
#include "state/eyeState.h"

BLEController bleController;
DisplayController *display;
EyeState eye;


void setup() {
	Serial.begin(115200);
	while (!Serial);
	pinMode(IR_PIN, INPUT);
	// bleController.init();
	display = new DisplayController();
	display->init();
}

bool isBoop, isOverrideEye = false;

void loop() {
	FastLED_Pixel_Buff->dimAll(200);
	drawColorTest();
	drawNose(noseDefault);
	drawMouth(mouthDefault);
	// bleController.start();
	isBoop = !digitalRead(IR_PIN);
	if (isBoop) {
		eye.setBoop();
	}
	else {
		eye.setIdle();
	}
	eye.update();
	FastLED_Pixel_Buff->show();
	// delay(25);
}