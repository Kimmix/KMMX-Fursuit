/**
 * FFT Performance Comparison: ArduinoFFT vs ESP-DSP
 *
 * This sketch compares the performance and output of two FFT libraries:
 * - ArduinoFFT (used in your current Viseme implementation)
 * - ESP-DSP (optimized ESP32 library)
 *
 * Hardware Setup:
 * - I2S Microphone connected to ESP32
 * - WS (Word Select): GPIO 10
 * - SD (Serial Data): GPIO 12
 * - SCK (Serial Clock): GPIO 11
 */

#include <Arduino.h>
#include <driver/i2s.h>
#include <arduinoFFT.h>
#include "esp_dsp.h"

// I2S Configuration
#define I2S_WS 10
#define I2S_SD 12
#define I2S_SCK 11
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

double noiseThreshold = 400;
const float alpha = 0.2;

// ArduinoFFT arrays
double vReal[samples];
double vImaginary[samples];
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImaginary, samples, sampleRate);

// ESP-DSP arrays
float fftInput[2 * samples];  // Interleaved complex data [real0, imag0, real1, imag1, ...]
int16_t i2sBuffer[samples];

// Performance counters
unsigned long arduinoFFTTime = 0;
unsigned long espDSPTime = 0;
int comparisonCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n===========================================");
  Serial.println("FFT Performance Comparison");
  Serial.println("ArduinoFFT vs ESP-DSP");
  Serial.println("===========================================\n");

  // Initialize I2S
  initI2S();

  // Initialize ESP-DSP FFT
  dsps_fft2r_init_fc32(NULL, samples);

  Serial.println("Libraries initialized");
  Serial.println("Starting comparison...\n");

  delay(1000);
}

void loop() {
  // Read audio samples once
  readI2SSamples();

  Serial.println("\n--- New Measurement ---");

  // Test ArduinoFFT
  unsigned long startArduino = micros();
  analyzeWithArduinoFFT();
  arduinoFFTTime = micros() - startArduino;

  // Test ESP-DSP
  unsigned long startESP = micros();
  analyzeWithESPDSP();
  espDSPTime = micros() - startESP;

  // Print performance comparison
  comparisonCount++;
  printPerformanceComparison();

  delay(1000);  // Wait 1 second between comparisons
}

void initI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = (uint32_t)sampleRate,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = samples,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);
  i2s_start(I2S_PORT);

  Serial.println("I2S Microphone initialized");
}

void readI2SSamples() {
  size_t bytes_read = 0;
  i2s_read(I2S_PORT, i2sBuffer, samples * sizeof(int16_t), &bytes_read, portMAX_DELAY);
}

void analyzeWithArduinoFFT() {
  // Apply exponential smoothing and copy to FFT array
  float smoothedValue = 0;
  for (int i = 0; i < samples; i++) {
    smoothedValue = alpha * i2sBuffer[i] + (1 - alpha) * smoothedValue;
    vReal[i] = smoothedValue;
    vImaginary[i] = 0;
  }

  // Perform FFT
  FFT.dcRemoval();
  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();

  // Analyze visemes
  double ah_amplitude = 0, ee_amplitude = 0, oh_amplitude = 0, oo_amplitude = 0, th_amplitude = 0;

  for (int i = 4; i < samples / 2; i++) {
    double freq = i * ((sampleRate / 2.0) / (samples / 2.0));
    double amplitude = abs(vImaginary[i]);

    if (freq >= AHFreqMin && freq <= AHFreqMax) ah_amplitude += amplitude;
    if (freq >= EEFreqMin && freq <= EEFreqMax) ee_amplitude += amplitude;
    if (freq >= OHFreqMin && freq <= OHFreqMax) oh_amplitude += amplitude;
    if (freq >= OOFreqMin && freq <= OOFreqMax) oo_amplitude += amplitude;
    if (freq >= THFreqMin && freq <= THFreqMax) th_amplitude += amplitude;
  }

  // Normalize
  ah_amplitude *= 0.6;
  ee_amplitude *= 1.0;
  oh_amplitude *= 1.8;
  oo_amplitude *= 2.2;
  th_amplitude *= 2.6;

  double max_amplitude = max(max(ah_amplitude, ee_amplitude), max(max(oh_amplitude, oo_amplitude), th_amplitude));
  double avg_amplitude = (ah_amplitude + ee_amplitude + oh_amplitude + oo_amplitude + th_amplitude) / 5.0;

  Serial.println("\n[ArduinoFFT Results]");
  Serial.print("  AH: "); Serial.print(ah_amplitude, 1);
  Serial.print(" | EE: "); Serial.print(ee_amplitude, 1);
  Serial.print(" | OH: "); Serial.print(oh_amplitude, 1);
  Serial.print(" | OO: "); Serial.print(oo_amplitude, 1);
  Serial.print(" | TH: "); Serial.println(th_amplitude, 1);
  Serial.print("  Max: "); Serial.print(max_amplitude, 1);
  Serial.print(" | Avg: "); Serial.println(avg_amplitude, 1);

  String dominant = getDominantViseme(ah_amplitude, ee_amplitude, oh_amplitude, oo_amplitude, th_amplitude);
  Serial.print("  Dominant: "); Serial.println(dominant);
}

void analyzeWithESPDSP() {
  // Apply exponential smoothing and copy to ESP-DSP format
  float smoothedValue = 0;
  for (int i = 0; i < samples; i++) {
    smoothedValue = alpha * i2sBuffer[i] + (1 - alpha) * smoothedValue;
    fftInput[2 * i] = smoothedValue;      // Real part
    fftInput[2 * i + 1] = 0.0f;           // Imaginary part
  }

  // Apply Hamming window
  dsps_wind_hann_f32(fftInput, samples);

  // Perform FFT
  dsps_fft2r_fc32(fftInput, samples);
  dsps_bit_rev_fc32(fftInput, samples);
  dsps_cplx2reC_fc32(fftInput, samples);

  // Analyze visemes
  double ah_amplitude = 0, ee_amplitude = 0, oh_amplitude = 0, oo_amplitude = 0, th_amplitude = 0;

  for (int i = 4; i < samples / 2; i++) {
    double freq = i * ((sampleRate / 2.0) / (samples / 2.0));

    // Calculate magnitude from complex FFT result
    float magnitude = sqrtf(fftInput[2 * i] * fftInput[2 * i] +
                           fftInput[2 * i + 1] * fftInput[2 * i + 1]);

    if (freq >= AHFreqMin && freq <= AHFreqMax) ah_amplitude += magnitude;
    if (freq >= EEFreqMin && freq <= EEFreqMax) ee_amplitude += magnitude;
    if (freq >= OHFreqMin && freq <= OHFreqMax) oh_amplitude += magnitude;
    if (freq >= OOFreqMin && freq <= OOFreqMax) oo_amplitude += magnitude;
    if (freq >= THFreqMin && freq <= THFreqMax) th_amplitude += magnitude;
  }

  // Normalize
  ah_amplitude *= 0.6;
  ee_amplitude *= 1.0;
  oh_amplitude *= 1.8;
  oo_amplitude *= 2.2;
  th_amplitude *= 2.6;

  double max_amplitude = max(max(ah_amplitude, ee_amplitude), max(max(oh_amplitude, oo_amplitude), th_amplitude));
  double avg_amplitude = (ah_amplitude + ee_amplitude + oh_amplitude + oo_amplitude + th_amplitude) / 5.0;

  Serial.println("\n[ESP-DSP Results]");
  Serial.print("  AH: "); Serial.print(ah_amplitude, 1);
  Serial.print(" | EE: "); Serial.print(ee_amplitude, 1);
  Serial.print(" | OH: "); Serial.print(oh_amplitude, 1);
  Serial.print(" | OO: "); Serial.print(oo_amplitude, 1);
  Serial.print(" | TH: "); Serial.println(th_amplitude, 1);
  Serial.print("  Max: "); Serial.print(max_amplitude, 1);
  Serial.print(" | Avg: "); Serial.println(avg_amplitude, 1);

  String dominant = getDominantViseme(ah_amplitude, ee_amplitude, oh_amplitude, oo_amplitude, th_amplitude);
  Serial.print("  Dominant: "); Serial.println(dominant);
}

String getDominantViseme(double ah, double ee, double oh, double oo, double th) {
  double maxVal = max(max(ah, ee), max(max(oh, oo), th));
  if (maxVal == ah) return "AH";
  else if (maxVal == ee) return "EE";
  else if (maxVal == oh) return "OH";
  else if (maxVal == oo) return "OO";
  else if (maxVal == th) return "TH";
  return "NONE";
}

void printPerformanceComparison() {
  Serial.println("\n[Performance Comparison]");
  Serial.print("  ArduinoFFT: "); Serial.print(arduinoFFTTime); Serial.println(" µs");
  Serial.print("  ESP-DSP:    "); Serial.print(espDSPTime); Serial.println(" µs");

  float speedup = (float)arduinoFFTTime / (float)espDSPTime;
  Serial.print("  Speedup:    "); Serial.print(speedup, 2); Serial.println("x");

  if (espDSPTime < arduinoFFTTime) {
    float improvement = ((float)(arduinoFFTTime - espDSPTime) / (float)arduinoFFTTime) * 100.0;
    Serial.print("  ESP-DSP is "); Serial.print(improvement, 1); Serial.println("% faster");
  } else {
    float improvement = ((float)(espDSPTime - arduinoFFTTime) / (float)espDSPTime) * 100.0;
    Serial.print("  ArduinoFFT is "); Serial.print(improvement, 1); Serial.println("% faster");
  }

  Serial.println("\n  Summary after " + String(comparisonCount) + " measurements:");
  Serial.print("  Average ArduinoFFT: "); Serial.print(arduinoFFTTime); Serial.println(" µs");
  Serial.print("  Average ESP-DSP:    "); Serial.print(espDSPTime); Serial.println(" µs");
  Serial.println("===========================================");
}
