#include <Arduino.h>
#include <MH_BMI160.h>
#include "Icons.h"
#include "Devices/LEDMatrixDisplay.h"
#include "Devices/Bluetooth.h"
#include "state/eyeState.h"
#include "state/mouthState.h"

#define IR_PIN 36
bool isBoop;

MH_BMI160 bmi160;
DisplayController display;

EyeState eyeState(&display, &bmi160);
MouthState mouthState(&display);
BluetoothController ble(&display);

void setup() {
	Serial.begin(115200);
	while (!Serial);
	ble.init();
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(IR_PIN, INPUT);
	randomSeed(analogRead(0));
	delay(100);
	bmi160.softReset();
	bmi160.I2cInit(0x69);
	delay(100);
}


void loop() {
	ble.update();
	display.render();
	display.drawColorTest();
	display.drawNose(noseDefault);
	isBoop = !digitalRead(IR_PIN);
	if (isBoop) {
		eyeState.setBoop();
		mouthState.setTalk();
	}
	eyeState.update();
	mouthState.update();
}