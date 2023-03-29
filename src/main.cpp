#include <Arduino.h>
#include <esp_random.h>
#include "Icons.h"
#include "Devices/LEDMatrixDisplay.h"
#include "Devices/Bluetooth.h"
#include "Devices/Microphone.h"
#include "state/eyeState.h"
#include "state/mouthState.h"

#define IR_PIN 36
bool isBoop;

DisplayController display;
Microphone microphone;

EyeState eyeState(&display);
MouthState mouthState(&display, &microphone);
// BluetoothController ble(&display);

TaskHandle_t Task1;
void Task1code(void* parameter) {
	while (true) {
		if (isBoop) {
			mouthState.setBoop();
		}
		mouthState.update();
	}
}

void setup() {
	Serial.begin(115200);
	while (!Serial);
	microphone.init();
	// ble.init();
	// pinMode(LED_BUILTIN, OUTPUT);
	pinMode(IR_PIN, INPUT);
	randomSeed(analogRead(14));
	xTaskCreatePinnedToCore(Task1code, "Task1", 10000, NULL, 0, &Task1, 0);
}

uint16_t fps = 0;
unsigned long fps_timer;
void loop() {
	// ble.update();
	// display.render();
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