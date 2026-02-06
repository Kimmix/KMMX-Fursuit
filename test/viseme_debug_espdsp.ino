/**
 * ESP32 I2S Microphone Viseme FFT Debug Tool (ESP-DSP Version)
 *
 * This sketch reads audio from an I2S microphone, performs FFT analysis using ESP-DSP,
 * and outputs the viseme frequency amplitudes for debugging.
 *
 * Hardware Setup:
 * - I2S Microphone connected to ESP32
 * - WS (Word Select): GPIO 17
 * - SD (Serial Data): GPIO 18
 * - SCK (Serial Clock): GPIO 5
 */

#include <Arduino.h>
#include <driver/i2s.h>
#include "esp_dsp.h"

// I2S Configuration
#define I2S_WS 17
#define I2S_SD 18
#define I2S_SCK 5
#define I2S_PORT I2S_NUM_0

// FFT Configuration
const double sampleRate = 8000;
const int samples = 256;

// Viseme Frequency Ranges
const int AHFreqMin = 600;
const int AHFreqMax = 1200;
const int EEFreqMin = 1000;
const int EEFreqMax = 2000;
const int OHFreqMin = 1800;
const int OHFreqMax = 2800;
const int OOFreqMin = 2600;
const int OOFreqMax = 3600;
const int THFreqMin = 3400;
const int THFreqMax = 4000;

// Noise threshold
double noiseThreshold = 400;

// Smoothing factor
const float alpha = 0.2;

// ESP-DSP FFT arrays (interleaved complex: [real0, imag0, real1, imag1, ...])
float fftInput[2 * samples];
int16_t i2sBuffer[samples];

void initI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = (uint32_t)sampleRate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = samples,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0};

    i2s_pin_config_t pin_config = {
        .bck_io_num = 11,  // BCLK
        .ws_io_num = 10,   // LRCLK
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = 12  // SD
    };

    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
    i2s_zero_dma_buffer(I2S_PORT);
    i2s_start(I2S_PORT);
}

void readI2SSamples() {
    size_t bytes_read = 0;

    // Read from I2S
    i2s_read(I2S_PORT, i2sBuffer, samples * sizeof(int16_t), &bytes_read, portMAX_DELAY);

    // Apply exponential smoothing and copy to FFT array (interleaved format)
    float smoothedValue = 0;
    for (int i = 0; i < samples; i++) {
        smoothedValue = alpha * i2sBuffer[i] + (1 - alpha) * smoothedValue;
        fftInput[2 * i] = smoothedValue;      // Real part
        fftInput[2 * i + 1] = 0.0f;           // Imaginary part
    }
}

void performFFT() {
    // Apply Hamming window
    dsps_wind_hann_f32(fftInput, samples);

    // Perform FFT
    dsps_fft2r_fc32(fftInput, samples);

    // Bit reversal
    dsps_bit_rev_fc32(fftInput, samples);

    // Convert to real spectrum
    dsps_cplx2reC_fc32(fftInput, samples);
}

void analyzeVisemes() {
    // Initialize amplitudes
    double ah_amplitude = 0;
    double ee_amplitude = 0;
    double oh_amplitude = 0;
    double oo_amplitude = 0;
    double th_amplitude = 0;

    // Sum amplitudes in each frequency range
    for (int i = 4; i < samples / 2; i++) {
        double freq = i * ((sampleRate / 2.0) / (samples / 2.0));

        // Calculate magnitude from complex FFT result
        float magnitude = sqrtf(fftInput[2 * i] * fftInput[2 * i] +
                               fftInput[2 * i + 1] * fftInput[2 * i + 1]);

        if (freq >= AHFreqMin && freq <= AHFreqMax) {
            ah_amplitude += magnitude;
        }
        if (freq >= EEFreqMin && freq <= EEFreqMax) {
            ee_amplitude += magnitude;
        }
        if (freq >= OHFreqMin && freq <= OHFreqMax) {
            oh_amplitude += magnitude;
        }
        if (freq >= OOFreqMin && freq <= OOFreqMax) {
            oo_amplitude += magnitude;
        }
        if (freq >= THFreqMin && freq <= THFreqMax) {
            th_amplitude += magnitude;
        }
    }

    // Normalize viseme amplitudes (same as original code)
    ah_amplitude *= 0.6;
    ee_amplitude *= 1.0;
    oh_amplitude *= 1.8;
    oo_amplitude *= 2.2;
    th_amplitude *= 2.6;

    // Calculate min, max, avg
    double min_amplitude = min(min(ah_amplitude, ee_amplitude), min(min(oh_amplitude, oo_amplitude), th_amplitude));
    double max_amplitude = max(max(ah_amplitude, ee_amplitude), max(max(oh_amplitude, oo_amplitude), th_amplitude));
    double avg_amplitude = (ah_amplitude + ee_amplitude + oh_amplitude + oo_amplitude + th_amplitude) / 5.0;

    // Calculate loudness level (0-20)
    double loudness_ratio = max_amplitude / avg_amplitude;
    int loudness_level = map(loudness_ratio * 100, 60, 280, 1, 20);
    loudness_level = constrain(loudness_level, 0, 20);

    // Apply noise threshold
    if (max_amplitude <= noiseThreshold) {
        loudness_level = 0;
    }

    // Determine dominant viseme
    String dominant = "NONE";
    double maxVal = max(max(ah_amplitude, ee_amplitude), max(max(oh_amplitude, oo_amplitude), th_amplitude));
    if (maxVal == ah_amplitude)
        dominant = "AH";
    else if (maxVal == ee_amplitude)
        dominant = "EE";
    else if (maxVal == oh_amplitude)
        dominant = "OH";
    else if (maxVal == oo_amplitude)
        dominant = "OO";
    else if (maxVal == th_amplitude)
        dominant = "TH";

    // Print debug output (formatted for Serial Plotter)
    int max_threshold = 6000;
    Serial.print("Max_THRESHOLD:");
    Serial.print(max_threshold);
    Serial.print(",noiseThreshold:");
    Serial.print(noiseThreshold);
    Serial.print(",AH:");
    Serial.print(ah_amplitude > max_threshold ? max_threshold : ah_amplitude);
    Serial.print(",EE:");
    Serial.print(ee_amplitude > max_threshold ? max_threshold : ee_amplitude);
    Serial.print(",OH:");
    Serial.print(oh_amplitude > max_threshold ? max_threshold : oh_amplitude);
    Serial.print(",OO:");
    Serial.print(oo_amplitude > max_threshold ? max_threshold : oo_amplitude);
    Serial.print(",TH:");
    Serial.print(th_amplitude > max_threshold ? max_threshold : th_amplitude);
    Serial.print(",Level:");
    Serial.print(loudness_level * 100);
    Serial.print(",Max:");
    Serial.print(max_amplitude > max_threshold ? max_threshold : max_amplitude);
    Serial.print(",AVG_AMP:");
    Serial.print(avg_amplitude > max_threshold ? max_threshold : avg_amplitude);
    Serial.print(",Dominant:");
    Serial.println(dominant);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("ESP32 I2S Microphone Viseme FFT Debug (ESP-DSP)");
    Serial.println("================================================");

    // Initialize I2S
    initI2S();

    // Initialize ESP-DSP FFT
    dsps_fft2r_init_fc32(NULL, samples);

    Serial.println("I2S Microphone initialized");
    Serial.println("ESP-DSP FFT initialized");
    Serial.println("Starting FFT analysis...\n");

    // Print header for serial plotter
    Serial.println("Max_THRESHOLD,noiseThreshold,AH,EE,OH,OO,TH,Level,Max,AVG_AMP");
}

void loop() {
    // Read audio samples
    readI2SSamples();

    // Perform FFT
    performFFT();

    // Analyze viseme frequencies
    analyzeVisemes();

    delay(50);  // Small delay between readings
}
