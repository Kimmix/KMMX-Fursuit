#include "I2SMicrophone.h"

const i2s_config_t I2SMicrophone::i2sConfig = {
  .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
  .sample_rate = 16000,
  .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
  .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
  .communication_format = I2S_COMM_FORMAT_I2S_MSB,
  .dma_buf_count = 2,
  .dma_buf_len = 1024 / 2,
  .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1
};

const i2s_pin_config_t I2SMicrophone::i2sPinConfig = {
  .bck_io_num = 26,
  .ws_io_num = 25,
  .data_out_num = -1,
  .data_in_num = 34
};

I2SMicrophone::I2SMicrophone(int bckPin, int wsPin, int dataPin, int sampleRate, int bufferSize) {
    _bckPin = bckPin;
    _wsPin = wsPin;
    _dataPin = dataPin;
    _sampleRate = sampleRate;
    _bufferSize = bufferSize;
}

void I2SMicrophone::begin() {
    i2s_driver_install(I2S_NUM_0, &i2sConfig, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &i2sPinConfig);
    i2s_set_sample_rates(I2S_NUM_0, _sampleRate);
}

void I2SMicrophone::read(uint8_t* dataBuffer, size_t bufferSize) {
    size_t bytesRead = 0;
    i2s_read(I2S_NUM_0, dataBuffer, bufferSize, &bytesRead, portMAX_DELAY);
}
