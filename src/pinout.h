// ESP32S3 Custom board
#define LED_BUILTIN 45
#define RANDOM_PIN 2  // Use any usused ADC pin for true random
// I2C
#define SDA 9
#define SCL 3
// I2S
#define I2S_WS 10
#define I2S_SD 12
#define I2S_SCK 11
// HUB75 pin
#define R1 4
#define G1 5
#define BL1 6
#define R2 7
#define G2 15
#define BL2 16
#define CH_A 18
#define CH_B 8
#define CH_C 19
#define CH_D 20
#define CH_E 17  // assign to any available pin if using two panels or 64x64 panels with 1/32 scan
#define CLK 41
#define LAT 40
#define OE 39
// Other
#define ARGB 14
#define ARGB_NUM 24
#define LED_PWM 21
#define IR_PIN GPIO_NUM_35
