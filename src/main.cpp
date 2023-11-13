#include <Arduino.h>
#include <ArduinoBLE.h>
#include <esp_random.h>
#include <esp_now.h>
#include <WiFi.h>

#include "pinout.h"
#include "helperFunction.h"
#include "Controller/controller.h"

Controller controller;
BLEService protoService("c1449275-bf34-40ab-979d-e34a1fdbb129");
BLEByteCharacteristic displayBrightnessCharacteristic("9fdfd124-966b-44f7-8331-778c4d1512fc", BLERead | BLEWrite);
BLEByteCharacteristic eyeStateCharacteristic("49a36bb2-1c66-4e5c-8ff3-28e55a64beb3", BLERead | BLEWrite);
BLEByteCharacteristic visemeCharacteristic("493d06f3-0fa0-4a90-88f1-ebaed0da9b80", BLERead | BLEWrite);

//? ------------------------ Blueooth ------------------------
static void blePeripheralConnectHandler(BLEDevice central) {
    Serial.print(F("Connected event, central: "));
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH);
}
// On bluetooth disconnected
static void blePeripheralDisconnectHandler(BLEDevice central) {
    Serial.print(F("Disconnected event, central: "));
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
void setupBLE() {
    Serial.println(F("Booting BLE..."));
    pinMode(LED_BUILTIN, OUTPUT);
    if (!BLE.begin()) {
        Serial.println(F("--------- Failed to initialize BLE! ---------"));
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
    Serial.println(F("Bluetooth® device active, waiting for connections..."));
}

//? ------------------------ ESP NOW ------------------------
uint8_t masterMacAddress[] = {0x84, 0xFC, 0xE6, 0x00, 0x26, 0x7C};
static const char* PMK_KEY_STR = "NHkeBaL5YkoAUsi6";
static const char* LMK_KEY_STR = "eYF8CUjnkFq3Ke5f";

typedef struct struct_message {
    uint16_t debugLed;
    int b;
} struct_message;
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
    memcpy(&myData, incomingData, sizeof(myData));
    // Serial.print("Bytes received: ");
    // Serial.print(len);
    // Serial.print(", Int: ");
    // Serial.println(myData.b);
    // Serial.println();
    controller.updatePixelPosition(myData.debugLed);
    controller.recieveEspNow(myData.b);
}

void setupEspNow() {
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    // Set the PMK key
    esp_now_set_pmk((uint8_t*)PMK_KEY_STR);

    // Register the master as peer
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, masterMacAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = true;
    // Setting the master device LMK key
    for (uint8_t i = 0; i < 16; i++) {
        peerInfo.lmk[i] = LMK_KEY_STR[i];
    }

    // Add master as peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);
}

//! -------------------- Main Setup --------------------
void setup() {
    Wire.begin(SDA,SCL);
    Serial.begin(115200);
    while (!Serial) delay(400);
    randomSeed(analogRead(RANDOM_PIN));
    controller.setupSensors();
    setupBLE();
    // setupEspNow();
}

void loop() {
    BLE.poll();
    controller.update();
    // showFPS();
}
