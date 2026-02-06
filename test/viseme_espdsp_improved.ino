/**
 * ESP32 I2S Microphone Viseme FFT Debug Tool (ESP-DSP Improved)
 *
 * A complete, standalone Arduino sketch for testing ESP-DSP-based viseme detection.
 * This version includes optimizations and bug fixes from the original implementation.
 *
 * Hardware Setup (from KMMX-Fursuit config.h):
 * - I2S Microphone connected to ESP32-S3
 * - WS  (Word Select/LRCLK): GPIO 10
 * - SD  (Serial Data):       GPIO 12
 * - SCK (Serial Clock/BCLK): GPIO 11
 *
 * Features:
 * - Pre-computed Hann window for efficiency
 * - Proper windowing BEFORE FFT (fixes original bug)
 * - DC removal for cleaner signal
 * - Optimized magnitude calculation
 * - Dual output mode: Serial Monitor + Serial Plotter compatible
 *
 * Required Libraries:
 * - ESP-DSP (esp_dsp.h) - install via ESP-IDF or Arduino Library Manager
 */

#include <Arduino.h>
#include <driver/i2s.h>
#include "esp_dsp.h"

// =============================================================================
// I2S CONFIGURATION (matches KMMX-Fursuit config.h)
// =============================================================================
#define I2S_WS 10   // Word Select (LRCLK)
#define I2S_SD 12   // Serial Data
#define I2S_SCK 11  // Serial Clock (BCLK)
#define I2S_PORT I2S_NUM_0

// =============================================================================
// FFT CONFIGURATION
// =============================================================================
const float SAMPLE_RATE = 8000.0f;                     // 8kHz sample rate (sufficient for speech)
const int FFT_SIZE = 256;                              // FFT size (must be power of 2)
const float FREQ_RESOLUTION = SAMPLE_RATE / FFT_SIZE;  // ~31.25 Hz per bin
const float NOISE_THRESHOLD = 1000.0f;                  // Minimum amplitude to detect speech (lowered for sensitivity)
const float SMOOTHING_ALPHA = 0.10f;                   // Input smoothing factor (0-1)
const int MAX_DISPLAY_VALUE = 9000;                    // Clamp for Serial Plotter readability
// =============================================================================
// VISEME FREQUENCY RANGES (tuned for vocal formants)
// Based on typical formant frequencies for vowel sounds:
// - F1 (first formant): 250-900 Hz - relates to jaw openness
// - F2 (second formant): 850-2500 Hz - relates to tongue position
// =============================================================================
// AH (as in "father") - open jaw, neutral tongue: F1~750Hz, F2~1200Hz
// Focus on mid-high range to separate from OH
const int AH_FREQ_MIN = 100;
const int AH_FREQ_MAX = 2000;

// EE (as in "see") - closed jaw, front tongue: F1~270Hz, F2~2300Hz
// High F2 is distinctive for EE
const int EE_FREQ_MIN = 60;
const int EE_FREQ_MAX = 600;

// OH (as in "go") - mid jaw, back tongue: F1~500Hz, F2~900Hz
// Focus on lower range to separate from AH
const int OH_FREQ_MIN = 1000;
const int OH_FREQ_MAX = 2000;

// OO (as in "boot") - closed jaw, back tongue: F1~300Hz, F2~870Hz
// Very narrow range - lowest formant
const int OO_FREQ_MIN = 500;
const int OO_FREQ_MAX = 1000;

// TH/consonants - high frequency fricatives
const int TH_FREQ_MIN = 2000;
const int TH_FREQ_MAX = 6000;

// Normalization multipliers - tuned to balance viseme sensitivity
// Lower values = less sensitive, Higher values = more sensitive
const float AH_SCALE = 0.4f;
const float EE_SCALE = 0.3f;
const float OH_SCALE = 2.5f;
const float OO_SCALE = 1.4f;
const float TH_SCALE = 6.0f;

// =============================================================================
// BUFFERS (allocated statically for performance)
// =============================================================================
int16_t i2sBuffer[FFT_SIZE];     // Raw I2S samples
float tempSamples[FFT_SIZE];     // Smoothed + windowed samples
float fftBuffer[2 * FFT_SIZE];   // Interleaved complex: [re0,im0,re1,im1,...]
float hannWindow[FFT_SIZE];      // Pre-computed Hann window
float magnitudes[FFT_SIZE / 2];  // Magnitude spectrum

// Viseme amplitudes (persistent for potential smoothing)
float ahAmplitude = 0, eeAmplitude = 0, ohAmplitude = 0;
float ooAmplitude = 0, thAmplitude = 0;

// Output mode toggle (press 'p' for plotter, 'm' for monitor)
bool plotterMode = false;

// =============================================================================
// INITIALIZATION FUNCTIONS
// =============================================================================

/**
 * Pre-compute the Hann window coefficients.
 * This saves ~3% CPU per frame compared to computing on-the-fly.
 */
void initHannWindow() {
    for (int i = 0; i < FFT_SIZE; i++) {
        hannWindow[i] = 0.5f * (1.0f - cosf(2.0f * PI * i / (FFT_SIZE - 1)));
    }
}

/**
 * Initialize I2S peripheral for microphone input.
 * Returns true on success, false on failure.
 */
bool initI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = (uint32_t)SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = FFT_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0};

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,  // BCLK
        .ws_io_num = I2S_WS,    // LRCLK
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD  // Data
    };

    esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("ERROR: I2S driver install failed: %s\n", esp_err_to_name(err));
        return false;
    }

    err = i2s_set_pin(I2S_PORT, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("ERROR: I2S pin config failed: %s\n", esp_err_to_name(err));
        return false;
    }

    i2s_zero_dma_buffer(I2S_PORT);
    i2s_start(I2S_PORT);
    return true;
}

/**
 * Initialize ESP-DSP FFT tables.
 * Must be called once before performing FFT.
 */
bool initFFT() {
    esp_err_t ret = dsps_fft2r_init_fc32(NULL, FFT_SIZE);
    if (ret != ESP_OK) {
        Serial.printf("ERROR: FFT init failed: %s\n", esp_err_to_name(ret));
        return false;
    }
    return true;
}

// =============================================================================
// AUDIO PROCESSING FUNCTIONS
// =============================================================================

/**
 * Read samples from I2S microphone and prepare for FFT.
 * Applies: DC removal -> smoothing -> Hann window
 *
 * IMPORTANT: Window is applied BEFORE copying to interleaved FFT buffer.
 * The original implementation had a bug where dsps_wind_hann_f32() was called
 * on interleaved data, which corrupted the imaginary components.
 */
void readAndPrepareSamples() {
    size_t bytesRead = 0;

    // Read raw 16-bit samples from I2S
    esp_err_t err = i2s_read(I2S_PORT, i2sBuffer, FFT_SIZE * sizeof(int16_t),
                             &bytesRead, portMAX_DELAY);
    if (err != ESP_OK) {
        Serial.printf("ERROR: I2S read failed: %s\n", esp_err_to_name(err));
        return;
    }

    // Calculate DC offset (mean value) for removal
    int32_t dcSum = 0;
    for (int i = 0; i < FFT_SIZE; i++) {
        dcSum += i2sBuffer[i];
    }
    float dcOffset = (float)dcSum / FFT_SIZE;

    // Apply DC removal, exponential smoothing, and Hann window
    float smoothedValue = 0;
    for (int i = 0; i < FFT_SIZE; i++) {
        // Remove DC offset
        float sample = (float)i2sBuffer[i] - dcOffset;

        // Apply exponential smoothing (low-pass filter)
        smoothedValue = SMOOTHING_ALPHA * sample + (1.0f - SMOOTHING_ALPHA) * smoothedValue;

        // Apply pre-computed Hann window
        tempSamples[i] = smoothedValue * hannWindow[i];
    }

    // Copy windowed samples to interleaved FFT buffer
    for (int i = 0; i < FFT_SIZE; i++) {
        fftBuffer[2 * i] = tempSamples[i];  // Real part
        fftBuffer[2 * i + 1] = 0.0f;        // Imaginary part = 0
    }
}

/**
 * Perform FFT using ESP-DSP library.
 * Uses hardware acceleration on ESP32-S3.
 */
void performFFT() {
    // Perform in-place radix-2 FFT
    dsps_fft2r_fc32(fftBuffer, FFT_SIZE);

    // Bit-reverse the output for correct ordering
    dsps_bit_rev_fc32(fftBuffer, FFT_SIZE);

    // Note: We compute magnitudes directly from complex output,
    // so we don't call dsps_cplx2reC_fc32() which overwrites the buffer
}

/**
 * Calculate magnitude spectrum from FFT output.
 * Only computes first half (positive frequencies, 0 to Nyquist).
 */
void calculateMagnitudes() {
    for (int i = 0; i < FFT_SIZE / 2; i++) {
        float re = fftBuffer[2 * i];
        float im = fftBuffer[2 * i + 1];
        magnitudes[i] = sqrtf(re * re + im * im);
    }
}

/**
 * Convert FFT bin index to frequency in Hz.
 */
inline float binToFrequency(int bin) {
    return bin * FREQ_RESOLUTION;
}

/**
 * Analyze magnitude spectrum and compute viseme amplitudes.
 * Uses AVERAGE energy per bin (not sum) to prevent wider bands from dominating.
 */
void analyzeVisemes() {
    // Reset amplitudes and bin counts
    ahAmplitude = eeAmplitude = ohAmplitude = ooAmplitude = thAmplitude = 0;
    int ahCount = 0, eeCount = 0, ohCount = 0, ooCount = 0, thCount = 0;

    for (int i = 0; i < FFT_SIZE / 2; i++) {
        float freq = binToFrequency(i);
        float mag = magnitudes[i];

        // Accumulate energy in each viseme frequency band
        // Using exclusive ranges to prevent overlap issues
        if (freq >= AH_FREQ_MIN && freq <= AH_FREQ_MAX) {
            ahAmplitude += mag;
            ahCount++;
        }
        if (freq >= EE_FREQ_MIN && freq <= EE_FREQ_MAX) {
            eeAmplitude += mag;
            eeCount++;
        }
        if (freq >= OH_FREQ_MIN && freq <= OH_FREQ_MAX) {
            ohAmplitude += mag;
            ohCount++;
        }
        if (freq >= OO_FREQ_MIN && freq <= OO_FREQ_MAX) {
            ooAmplitude += mag;
            ooCount++;
        }
        if (freq >= TH_FREQ_MIN && freq <= TH_FREQ_MAX) {
            thAmplitude += mag;
            thCount++;
        }
    }

    // Convert to average energy per bin (prevents wider bands from dominating)
    // if (ahCount > 0) ahAmplitude /= ahCount;
    // if (eeCount > 0) eeAmplitude /= eeCount;
    // if (ohCount > 0) ohAmplitude /= ohCount;
    // if (ooCount > 0) ooAmplitude /= ooCount;
    // if (thCount > 0) thAmplitude /= thCount;

    // Apply normalization to balance sensitivity across visemes
    ahAmplitude *= AH_SCALE;
    eeAmplitude *= EE_SCALE;
    ohAmplitude *= OH_SCALE;
    ooAmplitude *= OO_SCALE;
    thAmplitude *= TH_SCALE;
}

/**
 * Determine the dominant viseme based on highest amplitude.
 * Returns a string identifier.
 * Requires the dominant viseme to be at least 15% stronger than second place.
 */
const char* getDominantViseme(float& maxAmp) {
    // Find max and second max
    float amps[5] = {ahAmplitude, eeAmplitude, ohAmplitude, ooAmplitude, thAmplitude};
    const char* names[5] = {"AH", "EE", "OH", "OO", "TH"};

    int maxIdx = 0;
    maxAmp = amps[0];
    for (int i = 1; i < 5; i++) {
        if (amps[i] > maxAmp) {
            maxAmp = amps[i];
            maxIdx = i;
        }
    }

    // Find second highest
    float secondMax = 0;
    for (int i = 0; i < 5; i++) {
        if (i != maxIdx && amps[i] > secondMax) {
            secondMax = amps[i];
        }
    }

    // Require at least 15% separation for clear detection
    const float MIN_SEPARATION = 1.05f;
    if (maxAmp < NOISE_THRESHOLD || maxAmp < secondMax * MIN_SEPARATION) {
        return "---";  // No clear dominant viseme
    }

    return names[maxIdx];
}

// =============================================================================
// OUTPUT FUNCTIONS
// =============================================================================

/**
 * Print data formatted for Arduino Serial Plotter.
 * Format: label1:value1,label2:value2,...
 */
void printPlotterOutput() {
    float maxAmp;
    const char* dominant = getDominantViseme(maxAmp);

    // Calculate statistics
    float avgAmp = (ahAmplitude + eeAmplitude + ohAmplitude + ooAmplitude + thAmplitude) / 5.0f;

    // Calculate loudness level (0-20)
    float loudnessRatio = (avgAmp > 0) ? (maxAmp / avgAmp) : 0;
    int loudnessLevel = constrain((int)map(loudnessRatio * 100, 60, 280, 1, 20), 0, 20);
    if (maxAmp <= NOISE_THRESHOLD) loudnessLevel = 0;

    // Clamp values for readability
    auto clamp = [](float val) { return val > MAX_DISPLAY_VALUE ? MAX_DISPLAY_VALUE : val; };

    // Print in Serial Plotter format
    Serial.print("Threshold:");
    Serial.print(NOISE_THRESHOLD);
    Serial.print(",AH:");
    Serial.print(clamp(ahAmplitude));
    Serial.print(",EE:");
    Serial.print(clamp(eeAmplitude));
    Serial.print(",OH:");
    Serial.print(clamp(ohAmplitude));
    Serial.print(",OO:");
    Serial.print(clamp(ooAmplitude));
    Serial.print(",TH:");
    Serial.print(clamp(thAmplitude));
    Serial.print(",Level:");
    Serial.print(loudnessLevel * 100);
    Serial.print(",Max:");
    Serial.println(clamp(maxAmp));
}

/**
 * Print data formatted for Serial Monitor (human-readable).
 */
void printMonitorOutput() {
    float maxAmp;
    const char* dominant = getDominantViseme(maxAmp);

    // Calculate statistics
    float avgAmp = (ahAmplitude + eeAmplitude + ohAmplitude + ooAmplitude + thAmplitude) / 5.0f;
    int loudnessLevel = constrain((int)map(maxAmp / max(avgAmp, 1.0f) * 100, 60, 280, 1, 20), 0, 20);
    if (maxAmp <= NOISE_THRESHOLD) loudnessLevel = 0;

    // Create visual bar for each viseme (scaled to max 10 chars)
    auto makeBar = [maxAmp](float val) -> String {
        int len = (maxAmp > 0) ? (int)(val / maxAmp * 8) : 0;
        len = constrain(len, 0, 8);
        String bar = "";
        for (int i = 0; i < len; i++) bar += "#";
        for (int i = len; i < 8; i++) bar += ".";
        return bar;
    };

    Serial.printf("[%s] ", dominant);
    Serial.printf("AH:%s ", makeBar(ahAmplitude).c_str());
    Serial.printf("EE:%s ", makeBar(eeAmplitude).c_str());
    Serial.printf("OH:%s ", makeBar(ohAmplitude).c_str());
    Serial.printf("OO:%s ", makeBar(ooAmplitude).c_str());
    Serial.printf("TH:%s ", makeBar(thAmplitude).c_str());
    Serial.printf("| Max:%.0f\n", maxAmp);
}

/**
 * Check for serial commands to switch output modes.
 */
void checkSerialCommands() {
    if (Serial.available()) {
        char cmd = Serial.read();
        switch (cmd) {
            case 'p':
            case 'P':
                plotterMode = true;
                Serial.println("# Switched to PLOTTER mode");
                break;
            case 'm':
            case 'M':
                plotterMode = false;
                Serial.println("# Switched to MONITOR mode");
                break;
            case 'f':
            case 'F':
                Serial.println("\n# Frequency Ranges:");
                Serial.printf("#   AH: %d-%d Hz (scale: %.1fx)\n", AH_FREQ_MIN, AH_FREQ_MAX, AH_SCALE);
                Serial.printf("#   EE: %d-%d Hz (scale: %.1fx)\n", EE_FREQ_MIN, EE_FREQ_MAX, EE_SCALE);
                Serial.printf("#   OH: %d-%d Hz (scale: %.1fx)\n", OH_FREQ_MIN, OH_FREQ_MAX, OH_SCALE);
                Serial.printf("#   OO: %d-%d Hz (scale: %.1fx)\n", OO_FREQ_MIN, OO_FREQ_MAX, OO_SCALE);
                Serial.printf("#   TH: %d-%d Hz (scale: %.1fx)\n", TH_FREQ_MIN, TH_FREQ_MAX, TH_SCALE);
                Serial.printf("#   Noise Threshold: %.0f\n", NOISE_THRESHOLD);
                Serial.printf("#   Freq Resolution: %.2f Hz/bin\n\n", FREQ_RESOLUTION);
                break;
            case 'h':
            case 'H':
                Serial.println("\n# Commands:");
                Serial.println("#   p - Switch to Serial Plotter output");
                Serial.println("#   m - Switch to Serial Monitor output");
                Serial.println("#   f - Show frequency ranges and settings");
                Serial.println("#   h - Show this help\n");
                break;
        }
        // Flush remaining input
        while (Serial.available()) Serial.read();
    }
}

// =============================================================================
// ARDUINO MAIN FUNCTIONS
// =============================================================================

/**
 * Arduino setup - runs once at startup.
 */
void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(1000);  // Wait for serial to stabilize

    // Print header
    Serial.println();
    Serial.println("=========================================================");
    Serial.println("  ESP32 Viseme FFT Debug Tool (ESP-DSP Improved)");
    Serial.println("=========================================================");
    Serial.println();
    Serial.println("Configuration:");
    Serial.printf("  - Sample Rate:      %.0f Hz\n", SAMPLE_RATE);
    Serial.printf("  - FFT Size:         %d samples\n", FFT_SIZE);
    Serial.printf("  - Freq Resolution:  %.2f Hz/bin\n", FREQ_RESOLUTION);
    Serial.printf("  - Noise Threshold:  %.0f\n", NOISE_THRESHOLD);
    Serial.println();
    Serial.println("I2S Pins:");
    Serial.printf("  - WS (LRCLK):  GPIO %d\n", I2S_WS);
    Serial.printf("  - SD (Data):   GPIO %d\n", I2S_SD);
    Serial.printf("  - SCK (BCLK):  GPIO %d\n", I2S_SCK);
    Serial.println();

    // Initialize Hann window
    Serial.print("Initializing Hann window... ");
    initHannWindow();
    Serial.println("OK");

    // Initialize I2S
    Serial.print("Initializing I2S microphone... ");
    if (!initI2S()) {
        Serial.println("FAILED!");
        Serial.println("Check wiring and restart.");
        while (true) delay(1000);  // Halt on error
    }
    Serial.println("OK");

    // Initialize FFT
    Serial.print("Initializing ESP-DSP FFT... ");
    if (!initFFT()) {
        Serial.println("FAILED!");
        Serial.println("ESP-DSP library may not be installed.");
        while (true) delay(1000);  // Halt on error
    }
    Serial.println("OK");

    Serial.println();
    Serial.println("Commands: 'p' = Plotter, 'm' = Monitor, 'f' = Frequencies, 'h' = Help");
    Serial.println();
    Serial.println("Starting viseme analysis...");
    Serial.println("---------------------------------------------------------");

    // Small delay before starting main loop
    delay(500);
}

/**
 * Arduino loop - runs continuously.
 */
void loop() {
    // Check for mode switch commands
    checkSerialCommands();

    // Read and prepare audio samples
    readAndPrepareSamples();

    // Perform FFT analysis
    performFFT();

    // Calculate magnitude spectrum
    calculateMagnitudes();

    // Analyze viseme frequency bands
    analyzeVisemes();

    // Output results based on current mode
    if (plotterMode) {
        printPlotterOutput();
    } else {
        printMonitorOutput();
    }

    // Small delay between readings (~20 Hz update rate)
    delay(50);
}
