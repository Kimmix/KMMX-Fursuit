#include <Arduino.h>
#include "Icons.h"
#include "controller/BLEController.h"
#include "controller/displayController.h"
#include "state/eyeState.h"

#define IR_PIN 36
bool isBoop;

// BLEController bleController;
DisplayController display;
EyeState eyeState(&display);

void setup() {
	Serial.begin(115200);
	while (!Serial);
	pinMode(IR_PIN, INPUT);
}


void loop() {
	display.clearScreen();
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
	display.render();
}