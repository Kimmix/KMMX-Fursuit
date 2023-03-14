#include <Arduino.h>
#include "Icons.h"
#include "Devices/LEDMatrixDisplay.h"
#include "Devices/Bluetooth.h"
#include "state/eyeState.h"
#include "state/mouthState.h"

#define IR_PIN 36
bool isBoop;

DisplayController display;
EyeState eyeState(&display);
MouthState mouthState(&display);
BluetoothController ble(&display);

void setup() {
	Serial.begin(115200);
	while (!Serial);
	Serial.println("starting...");
	ble.init();
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(IR_PIN, INPUT);
}


void loop() {
	ble.update();
	display.render();
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
}