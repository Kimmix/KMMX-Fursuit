#include <Arduino.h>
#include <ArduinoBLE.h>
#include <esp_random.h>

#include "helperFunction.h"
#include "Controller/controller.h"
#define LED_BUILTIN GPIO_NUM_2

Controller controller;
BLEService protoService("c1449275-bf34-40ab-979d-e34a1fdbb129");
BLEByteCharacteristic displayBrightnessCharacteristic("9fdfd124-966b-44f7-8331-778c4d1512fc", BLERead | BLEWrite);
BLEByteCharacteristic eyeStateCharacteristic("49a36bb2-1c66-4e5c-8ff3-28e55a64beb3", BLERead | BLEWrite);
BLEByteCharacteristic visemeCharacteristic("493d06f3-0fa0-4a90-88f1-ebaed0da9b80", BLERead | BLEWrite);

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

void displayBrightnessWritten(BLEDevice central, BLECharacteristic characteristic) {
    const uint8_t* data = characteristic.value();
    controller.setDisplayBrightness(static_cast<int>(*data));
}
void eyeStateWritten(BLEDevice central, BLECharacteristic characteristic) {
    const uint8_t* data = characteristic.value();
    controller.setEye(static_cast<int>(*data));
}
void visemeStateWritten(BLEDevice central, BLECharacteristic characteristic) {
    const uint8_t* data = characteristic.value();
    controller.setViseme(static_cast<int>(*data));
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

    // Define the BLE protoService  and characteristic
    BLE.setAdvertisedService(protoService);
    protoService.addCharacteristic(displayBrightnessCharacteristic);
    protoService.addCharacteristic(eyeStateCharacteristic);
    protoService.addCharacteristic(visemeCharacteristic);

    // Set default values for each characteristic
    displayBrightnessCharacteristic.setValue(controller.getDisplayBrightness());
    eyeStateCharacteristic.setValue(0x00);
    visemeCharacteristic.setValue(controller.getViseme());

    BLE.addService(protoService);
    BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
    BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

    // assign event handlers for characteristic
    displayBrightnessCharacteristic.setEventHandler(BLEWritten, displayBrightnessWritten);
    eyeStateCharacteristic.setEventHandler(BLEWritten, eyeStateWritten);
    visemeCharacteristic.setEventHandler(BLEWritten, visemeStateWritten);

    // Start advertising the BLE pService
    BLE.advertise();
    Serial.println("Bluetooth® device active, waiting for connections...");
}

//! -------------------- Main Setup --------------------
void setup() {
    Serial.begin(115200);
    while (!Serial) delay(400);
    pinMode(IR_PIN, INPUT);
    randomSeed(analogRead(RANDOM_PIN));
    setupBLE();
    controller.setupSensors();
}

void loop() {
    BLE.poll();
    controller.update();
    // showFPS();
}
