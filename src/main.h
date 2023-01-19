#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <FastLED.h>

#define BAUD_RATE 115200  // serial debug port baud rate
// Sensors
#define IR_PIN 36

// BLE
#define BLE_SERVICE_UUID "c1449275-bf34-40ab-979d-e34a1fdbb129"
#define BLE_CHARACTERISTIC_UUID "49a36bb2-1c66-4e5c-8ff3-28e55a64beb3"

// ---- P3 LED Matrix Panel ----
// HUB75E pinout
// R1 | G1
// B1 | GND
// R2 | G2
// B2 | E
//  A | B
//  C | D
// CLK| LAT
// OE | GND

/*  Default library pin configuration for the reference
  you can redefine only ones you need later on object creation
#define R1 25
#define G1 26
#define BL1 27
#define R2 14
#define G2 12
#define BL2 13
#define CH_A 23
#define CH_B 19
#define CH_C 5
#define CH_D 17
#define CH_E -1 // assign to any available pin if using panels with 1/32 scan
#define CLK 16
#define LAT 4
#define OE 15
*/
extern MatrixPanel_I2S_DMA* matrix;
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 32
#define PANELS_NUMBER 2
#define PANE_WIDTH PANEL_WIDTH* PANELS_NUMBER
#define PANE_HEIGHT PANEL_HEIGHT
#define NUM_LEDS PANE_WIDTH* PANE_HEIGHT
void buffclear(CRGB* buf);

// KMMX Custom draw function
void myColor(const uint8_t lightness,
             int row,
             uint8_t& r,
             uint8_t& g,
             uint8_t& b);
void drawGSBitmap(const uint8_t bitmap[]);
void drawEye(const uint8_t bitmap[]);
void drawNose(const uint8_t bitmap[]);
void drawMouth(const uint8_t bitmap[]);
void blink();
void boop(bool isBoop);