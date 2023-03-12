#include "main.h"
#include <Arduino.h>
#include "Icons.h"
#include "controller/BLEController.h"
#include "controller/displayController.h"
#include "state/eyeState.h"

BLEController bleController;
DisplayController* display;
EyeState eye(display);


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
	display->drawColorTest();
	display->drawNose(noseDefault);
	display->drawMouth(mouthDefault);
	// bleController.start();
	isBoop = !digitalRead(IR_PIN);
	if (isBoop) {
		eye.setBoop();
		display->drawMouth(mouthOpen);
	}
	eye.update();
	FastLED_Pixel_Buff->show();
	// delay(25);
}