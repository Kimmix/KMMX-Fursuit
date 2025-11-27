#include <Arduino.h>
#include <driver/i2s.h>
#include "esp_dsp.h"

// ====== CONFIG ======
#define SAMPLE_RATE 16000
#define FFT_SIZE 512
#define I2S_PORT I2S_NUM_0

// ====== Viseme Frequency Bands ======
const int AHFreqMin = 300;  // AH (open vowel)
const int AHFreqMax = 900;
const int EEFreqMin = 800;  // EE
const int EEFreqMax = 1600;
const int OHFreqMin = 1500;  // OH
const int OHFreqMax = 2300;
const int OOFreqMin = 2200;  // OO
const int OOFreqMax = 3100;
const int THFreqMin = 3000;  // TH (fricative)
const int THFreqMax = 4500;

// ====== Buffers ======
float real_buffer[FFT_SIZE];
float imag_buffer[FFT_SIZE];

// ====== Smoothing Variables ======
float smoothAH = 0, smoothEE = 0, smoothOH = 0, smoothOO = 0, smoothTH = 0;
float smoothFactor = 0.7;

// ====== I2S SETUP ======
void setupI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 128,
        .use_apll = false,
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = 42,  // BCLK
        .ws_io_num = 41,   // LRCLK
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = 2  // SD
    };

    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
}

// ====== Frequency → FFT bin helper ======
int freqToIndex(int freq) {
    return (int)((float)freq * FFT_SIZE / SAMPLE_RATE);
}

// ====== Band Energy Calculation ======
float bandEnergy(float* mag, int fMin, int fMax) {
    int iMin = freqToIndex(fMin);
    int iMax = freqToIndex(fMax);
    float sum = 0;
    for (int i = iMin; i <= iMax; i++) sum += mag[i];
    return sum / (iMax - iMin + 1);
}

// ====== SETUP ======
void setup() {
    Serial.begin(115200);
    setupI2S();

    // Initialize FFT tables
    dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);

    Serial.println("ESP-DSP Viseme Detector Started");
}

// ====== LOOP ======
void loop() {
    int32_t i2s_samples[FFT_SIZE];
    size_t bytes_read = 0;

    // Read audio samples
    i2s_read(I2S_PORT, (char*)i2s_samples, FFT_SIZE * sizeof(int32_t), &bytes_read, portMAX_DELAY);

    // Copy samples into float buffer
    for (int i = 0; i < FFT_SIZE; i++) {
        real_buffer[i] = (float)i2s_samples[i] / 2147483648.0f;  // Normalize 32-bit PCM
        imag_buffer[i] = 0.0f;
    }

    // Apply window function
    dsps_wind_hann_f32(real_buffer, FFT_SIZE);

    // Perform FFT
    dsps_fft2r_fc32(real_buffer, FFT_SIZE);
    dsps_bit_rev_fc32(real_buffer, FFT_SIZE);
    dsps_cplx2real_fc32(real_buffer, FFT_SIZE);

    // Compute magnitude
    float mag[FFT_SIZE / 2];
    for (int i = 0; i < FFT_SIZE / 2; i++) {
        float re = real_buffer[2 * i];
        float im = real_buffer[2 * i + 1];
        mag[i] = sqrtf(re * re + im * im);
    }

    // Compute viseme band energies
    float AH = bandEnergy(mag, AHFreqMin, AHFreqMax);
    float EE = bandEnergy(mag, EEFreqMin, EEFreqMax);
    float OH = bandEnergy(mag, OHFreqMin, OHFreqMax);
    float OO = bandEnergy(mag, OOFreqMin, OOFreqMax);
    float TH = bandEnergy(mag, THFreqMin, THFreqMax);

    // Smooth
    smoothAH = smoothFactor * smoothAH + (1 - smoothFactor) * AH;
    smoothEE = smoothFactor * smoothEE + (1 - smoothFactor) * EE;
    smoothOH = smoothFactor * smoothOH + (1 - smoothFactor) * OH;
    smoothOO = smoothFactor * smoothOO + (1 - smoothFactor) * OO;
    smoothTH = smoothFactor * smoothTH + (1 - smoothFactor) * TH;

    // Normalize
    float maxVal = max({smoothAH, smoothEE, smoothOH, smoothOO, smoothTH});
    if (maxVal < 1e-6f) maxVal = 1e-6f;
    smoothAH /= maxVal;
    smoothEE /= maxVal;
    smoothOH /= maxVal;
    smoothOO /= maxVal;
    smoothTH /= maxVal;

    // Determine dominant viseme
    float energies[5] = {smoothAH, smoothEE, smoothOH, smoothOO, smoothTH};
    const char* names[5] = {"AH", "EE", "OH", "OO", "TH"};
    int maxIdx = 0;
    for (int i = 1; i < 5; i++) {
        if (energies[i] > energies[maxIdx]) maxIdx = i;
    }

    // Output for Serial Plotter
    Serial.printf("AH:%.2f\tEE:%.2f\tOH:%.2f\tOO:%.2f\tTH:%.2f\tViseme:%s\n",
                  smoothAH, smoothEE, smoothOH, smoothOO, smoothTH, names[maxIdx]);
}
