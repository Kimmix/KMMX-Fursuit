#include <Arduino.h>
#include "Icons.h"
#include "controller/BLEController.h"
#include "controller/displayController.h"
#include "state/eyeState.h"

#define IR_PIN 36
bool isBoop;

BLEController bleController;
DisplayController display;
EyeState eyeState(&display);

void setup() {
	Serial.begin(115200);
	while (!Serial);
	pinMode(IR_PIN, INPUT);
	// bleController.init();
	display.init();
}


void loop() {
	FastLED_Pixel_Buff->dimAll(200);
	display.drawColorTest();
	display.drawNose(noseDefault);
	display.drawMouth(mouthDefault);
	// bleController.start();
	isBoop = !digitalRead(IR_PIN);
	if (isBoop) {
		eyeState.setBoop();
		display.drawMouth(mouthOpen);
	}
	eyeState.update();
	FastLED_Pixel_Buff->show();
}