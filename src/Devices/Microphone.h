#include <driver/i2s.h>

// Connections to I2S microphone
#define I2S_WS GPIO_NUM_44
#define I2S_SD GPIO_NUM_17
#define I2S_SCK GPIO_NUM_43

#define SAMPLE_RATE 8000
#define SAMPLES 256

// Use I2S Processor 0
#define I2S_PORT I2S_NUM_0

class Microphone {
public:
    Microphone() {};

    void init() {
        delay(500);
        Serial.println("Configuring I2S...");
        // Set up I2S Processor configuration
        const i2s_config_t i2s_config = {
          .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
          .sample_rate = SAMPLE_RATE,
          .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
          .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
          .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
          .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
          .dma_buf_count = 8,
          .dma_buf_len = SAMPLES,
        //   .use_apll = false
        };
        // Set I2S pin configuration
        const i2s_pin_config_t pin_config = {
          .bck_io_num = I2S_SCK,
          .ws_io_num = I2S_WS,
          .data_out_num = -1,
          .data_in_num = I2S_SD
        };
        esp_err_t err;
        err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
        if (err != ESP_OK) {
            Serial.printf("Failed installing driver: %d\n", err);
            while (true);
        }
        err = i2s_set_pin(I2S_PORT, &pin_config);
        if (err != ESP_OK) {
            Serial.printf("Failed setting pin: %d\n", err);
            while (true);
        }
        Serial.println("I2S driver installed.");
        delay(500);
    };

    void read(int16_t* buffer, size_t num_samples) {
        size_t bytes_read;
        i2s_read(I2S_PORT, (void*)buffer, num_samples * 2, &bytes_read, portMAX_DELAY);
    };
};
