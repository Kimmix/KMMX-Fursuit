#include <Arduino.h>
// #include <MH_BMI160.h>
#include "Icons.h"
#include "Devices/LEDMatrixDisplay.h"
#include "Devices/Bluetooth.h"
#include "state/eyeState.h"
#include "state/mouthState.h"

#define IR_PIN 36
bool isBoop;

// MH_BMI160 bmi160;
DisplayController display;

EyeState eyeState(&display);
MouthState mouthState(&display);
// BluetoothController ble(&display);

TaskHandle_t Task1;
void Task1code(void* parameter) {
	while(true) {
		if (isBoop) {
			mouthState.setBoop();
		}
		mouthState.update();
	}
}

void setup() {
	Serial.begin(115200);
	while (!Serial);
	// ble.init();
	// pinMode(LED_BUILTIN, OUTPUT);
	pinMode(IR_PIN, INPUT);
	randomSeed(analogRead(14));
	// delay(100);
	// bmi160.softReset();
	// bmi160.I2cInit(0x69);
	xTaskCreatePinnedToCore(
		Task1code, /* Function to implement the task */
		"Task1", /* Name of the task */
		10000,  /* Stack size in words */
		NULL,  /* Task input parameter */
		0,  /* Priority of the task */
		&Task1,  /* Task handle. */
		0); /* Core where the task should run */
}

uint16_t fps = 0;
unsigned long fps_timer;
void loop() {
	// ble.update();
	// display.render();
	// delay(25);
	// display.clearScreen();
	display.drawColorTest();
	display.drawNose(noseDefault);
	isBoop = !digitalRead(IR_PIN);
	if (isBoop) {
		eyeState.setBoop();
		mouthState.setBoop();
	}
	eyeState.update();
	// ++fps;
	// if (fps_timer + 1000 < millis()) {
	// 	Serial.printf_P(PSTR("Effect fps: %d\n"), fps);
	// 	fps_timer = millis();
	// 	fps = 0;
	// }
}