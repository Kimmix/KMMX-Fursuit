#include <Arduino.h>
#include "Icons.h"
#include "controller/BLEController.h"
#include "controller/displayController.h"
#include "state/eyeState.h"
#include "state/mouthState.h"

#define IR_PIN 36
bool isBoop;

// BLEController bleController;
DisplayController display;
EyeState eyeState(&display);
MouthState mouthState(&display);

void setup() {
	Serial.begin(115200);
	while (!Serial);
	pinMode(IR_PIN, INPUT);
}


void loop() {
	// bleController.start();
	display.clearScreen();
	display.drawColorTest();
	display.drawNose(noseDefault);
	isBoop = !digitalRead(IR_PIN);
	if (isBoop) {
		eyeState.setBoop();
		mouthState.setBoop();
	}
	eyeState.update();
	mouthState.update();
	display.render();
}