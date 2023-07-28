#include <Arduino.h>
#include <ArduinoBLE.h>
#include <esp_random.h>

#include "Controller/controller.h"
#define IR_PIN GPIO_NUM_35
#define LED_BUILTIN GPIO_NUM_2

Controller controller;

BLEService facialExpressionService("c1449275-bf34-40ab-979d-e34a1fdbb129");
BLEByteCharacteristic brightnessCharacteristic("49a36bb2-1c66-4e5c-8ff3-28e55a64beb3", BLERead | BLEWrite);

//? ------------------------ Blueooth 
static void blePeripheralConnectHandler(BLEDevice central) {
    Serial.print("Connected event, central: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH);
}
// On bluetooth disconnected
static void blePeripheralDisconnectHandler(BLEDevice central) {
    Serial.print("Disconnected event, central: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, LOW);
}

void switchCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
    Serial.print("Characteristic event, written: ");
    Serial.println(brightnessCharacteristic.value());
    controller.faceBoop();
    // display->setBrightnessValue(brightnessCharacteristic.value());
}
//? ------------- Blueooth Setup
void setupBLE() {
    Serial.println("Booting BLE...");
    pinMode(LED_BUILTIN, OUTPUT);
    if (!BLE.begin()) {
        Serial.println("failed to initialize BLE!");
        while (1)
            ;
    }
    BLE.setDeviceName("KMMX");
    BLE.setLocalName("KMMX-BLE");

    // Define the BLE facialExpressionService  and characteristic
    BLE.setAdvertisedService(facialExpressionService);
    facialExpressionService.addCharacteristic(brightnessCharacteristic);

    BLE.addService(facialExpressionService);
    BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
    BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

    // assign event handlers for characteristic
    brightnessCharacteristic.setEventHandler(BLEWritten, switchCharacteristicWritten);

    // Start advertising the BLE pService
    BLE.advertise();
    Serial.println("Bluetooth® device active, waiting for connections...");
}

uint16_t fps = 0;
unsigned long fps_timer;
void showFPS() {
    ++fps;
    if (millis() > fps_timer) {
        Serial.printf_P(PSTR("Effect fps: %d\n"), fps);
        fps_timer = millis() + 1000;
        fps = 0;
    }
}

//! -------------------- Main Setup -------------------- 
void setup() {
    Serial.begin(115200);
    while (!Serial) delay(400);
    setupBLE();
    controller.setupDevices();
}

void loop() {
    // showFPS();
    BLE.poll();
    if (!digitalRead(IR_PIN)) {
        controller.faceBoop();
    }
    controller.update();
}
