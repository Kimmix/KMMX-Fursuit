#include "main.h"
#include <Arduino.h>
#include <ArduinoBLE.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "Icons.h"

// BLE Service
BLEService kmmxBLEControl(BLE_SERVICE_UUID);
BLEByteCharacteristic controlCharacteristic(BLE_CHARACTERISTIC_UUID,
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
  Serial.print("Characteristic event, written: ");
  if (controlCharacteristic.value()) {
    Serial.println("LED on");
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    Serial.println("LED off");
    digitalWrite(LED_BUILTIN, LOW);
  }
}

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
  // ------ Setup Bluetooth Low Energy ------
  pinMode(LED_BUILTIN, OUTPUT);
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
  }
  BLE.setDeviceName("KMMX");
  BLE.setLocalName("KMMX-BLE");
  BLE.setAdvertisedService(kmmxBLEControl);
  kmmxBLEControl.addCharacteristic(controlCharacteristic);
  BLE.addService(kmmxBLEControl);
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  controlCharacteristic.setEventHandler(BLEWritten,
                                        switchCharacteristicWritten);
  controlCharacteristic.setValue(0);
  BLE.advertise();
  Serial.println(("Bluetooth® device active, waiting for connections..."));

  // ------ Setup P3 LED Matrix Pannel ------
  HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANELS_NUMBER);
  mxconfig.driver = HUB75_I2S_CFG::ICN2038S;
  matrix = new MatrixPanel_I2S_DMA(mxconfig);
  // matrix->setBrightness8(20);  // 0-255
  matrix->begin();

  Serial.begin(BAUD_RATE);
  // matrix->clearScreen();
  // drawGSBitmap(face_gs);
  drawEye(eyeDefault);
  drawNose(noseDefault);
  drawMouth(mouthDefault);
  drawColorTest();
}

void loop() {
  BLE.poll(); // Start BLE
  drawEye(eyeDefault);
  delay(6);
  drawEye(eyeBlink0);
  delay(7);
  drawEye(eyeBlink1);
  delay(7);
  drawEye(eyeBlink2);
  delay(8);
  drawEye(eyeBlink3);
  delay(9);
  drawEye(eyeBlink4);
  delay(9);
  drawEye(eyeBlink5);
  delay(10);
  drawEye(eyeBlink6);
  delay(13);
  drawEye(eyeBlink5);
  delay(10);
  drawEye(eyeBlink4);
  delay(10);
  drawEye(eyeBlink3);
  delay(10);
  drawEye(eyeBlink2);
  delay(11);
  drawEye(eyeBlink1);
  delay(12);
  drawEye(eyeBlink0);
  delay(15);
  drawEye(eyeDefault);
  delay(2000);
  // Serial.println("Fill screen: RED");
  // matrix->fillScreenRGB888(255, 0, 0);
  // delay(PATTERN_DELAY);
  // draw_eye(0, 0, myEye);
}