#include "I2SMicrophone.h"

I2SMicrophone::I2SMicrophone() {}

I2SMicrophone::~I2SMicrophone() {
    stop_i2s();
}

void I2SMicrophone::i2s_install(uint32_t sampleRate, int samples) {
    const i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = sampleRate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = samples,
        .use_apll = false};

    esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("Failed to install I2S driver: %s\n", esp_err_to_name(err));
    }
}

void I2SMicrophone::i2s_setpin() {
    const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = -1,
        .data_in_num = I2S_SD
    };

    esp_err_t err = i2s_set_pin(I2S_PORT, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("Failed to set I2S pins: %s\n", esp_err_to_name(err));
    }
}

void I2SMicrophone::init(uint32_t sampleRate, int samples) {
    Serial.println(F("Starting I2S mic..."));
    i2s_install(sampleRate, samples);
    i2s_setpin();
    start_i2s();
}

void I2SMicrophone::read(int16_t* buffer, int samples) {
    size_t bytes_read;
    esp_err_t err = i2s_read(I2S_PORT, (void*)buffer, samples * sizeof(int16_t), &bytes_read, portMAX_DELAY);
    if (err != ESP_OK) {
        Serial.printf("Failed to read I2S data: %s\n", esp_err_to_name(err));
    }
}

void I2SMicrophone::start_i2s() {
    i2s_zero_dma_buffer(I2S_PORT);
    esp_err_t err = i2s_start(I2S_PORT);
    if (err != ESP_OK) {
        Serial.printf("Failed to start I2S: %s\n", esp_err_to_name(err));
    }
}

void I2SMicrophone::stop_i2s() {
    esp_err_t err = i2s_stop(I2S_PORT);
    if (err != ESP_OK) {
        Serial.printf("Failed to stop I2S: %s\n", esp_err_to_name(err));
    }

    err = i2s_driver_uninstall(I2S_PORT);
    if (err != ESP_OK) {
        Serial.printf("Failed to uninstall I2S driver: %s\n", esp_err_to_name(err));
    }
}
