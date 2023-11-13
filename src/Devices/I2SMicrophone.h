#include <driver/i2s.h>

// Use I2S Processor 0
#define I2S_PORT I2S_NUM_0

class I2SMicrophone {
   private:
    void i2s_install(uint32_t sampleRate, int samples) {
        // Set up I2S Processor configuration
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
        i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    }

    void i2s_setpin() {
        // Set I2S pin configuration
        const i2s_pin_config_t pin_config = {
            .bck_io_num = I2S_SCK,
            .ws_io_num = I2S_WS,
            .data_out_num = -1,
            .data_in_num = I2S_SD};
        i2s_set_pin(I2S_PORT, &pin_config);
    }

   public:
    void init(uint32_t sampleRate, int samples) {
        Serial.println(F("Starting I2C mic..."));
        i2s_install(sampleRate, samples);
        i2s_setpin();
        start_i2s();
    };

    void read(int16_t* buffer, int samples) {
        size_t bytes_read;
        i2s_read(I2S_PORT, (void*)buffer, samples * 2, &bytes_read, portMAX_DELAY);
    };

    void start_i2s() {
        // Start I2S microphone
        i2s_zero_dma_buffer(I2S_NUM_0);
        i2s_start(I2S_PORT);
    }

    void stop_i2s() {
        // Stop I2S microphone
        i2s_stop(I2S_NUM_0);
        i2s_driver_uninstall(I2S_NUM_0);
    }
};
