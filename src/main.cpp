#include "main.h"
#include <Arduino.h>
#include <ArduinoBLE.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "Icons.h"

// BLE Service
BLEService kmmxBLEControl(BLE_SERVICE_UUID);
BLEByteCharacteristic brightnessCharacteristic(BLE_CHARACTERISTIC_UUID,
                                            BLERead | BLEWrite);
// On bluetooth connected
void blePeripheralConnectHandler(BLEDevice central) {
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
}
// On bluetooth disconnected
void blePeripheralDisconnectHandler(BLEDevice central) {
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
}
void switchCharacteristicWritten(BLEDevice central,
                                 BLECharacteristic characteristic) {
  // Serial.print("Characteristic event, written: ");
  // Serial.println(brightnessCharacteristic.value());
  matrix->setBrightness8(brightnessCharacteristic.value());  // 0-255
}

// *** Led Matrix ***
MatrixPanel_I2S_DMA* matrix = nullptr;
void buffclear(CRGB* buf) {
  memset(buf, 0x00, NUM_LEDS * sizeof(CRGB));  // flush buffer to black
}
void drawColorTest() {
  for (int i = 0; i < 64; i++) {
    uint8_t r, g, b;
    myColor(255, i, r, g, b);
    matrix->drawPixelRGB888(0, i, r, g, b);
  }
}

void setup() {
  pinMode(IR_PIN, INPUT);
  randomSeed(analogRead(0));
  // ------ Setup Bluetooth Low Energy ------
  pinMode(LED_BUILTIN, OUTPUT);
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
  }
  BLE.setDeviceName("KMMX");
  BLE.setLocalName("KMMX-BLE");
  BLE.setAdvertisedService(kmmxBLEControl);
  kmmxBLEControl.addCharacteristic(brightnessCharacteristic);
  BLE.addService(kmmxBLEControl);
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  brightnessCharacteristic.setEventHandler(BLEWritten,
                                        switchCharacteristicWritten);
  brightnessCharacteristic.setValue(0);
  BLE.advertise();
  Serial.println(("Bluetooth® device active, waiting for connections..."));

  // ------ Setup P3 LED Matrix Pannel ------
  HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANELS_NUMBER);
  mxconfig.driver = HUB75_I2S_CFG::ICN2038S;
  matrix = new MatrixPanel_I2S_DMA(mxconfig);
  matrix->begin();

  Serial.begin(BAUD_RATE);
  // matrix->clearScreen();
  // drawGSBitmap(face_gs);
  drawEye(eyeDefault);
  drawNose(noseDefault);
  drawMouth(mouthDefault);
  drawColorTest();
}

bool isBoop;

void loop() {
  BLE.poll();  // Start BLE
  isBoop = !digitalRead(IR_PIN);
  if (isBoop) {
    boop(isBoop);
  } else {
    blink();
  }
  // oFace();
  // Serial.println("Fill screen: RED");
  // matrix->fillScreenRGB888(255, 0, 0);
  // delay(PATTERN_DELAY);
  // draw_eye(0, 0, myEye);
}