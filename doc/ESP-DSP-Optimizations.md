# ESP-DSP Optimization Opportunities for KMMX-Fursuit

Based on analysis of `src/KMMXController` and `src/Devices` folders.

---

## Summary Table

| Priority | Component | Current | Optimization | Est. Improvement |
|----------|-----------|---------|--------------|------------------|
| 🎯 HIGH | Viseme FFT | ArduinoFFT (double) | ESP-DSP (float) | 3-5x faster |
| 🎯 HIGH | Frequency Analysis | Loop + if checks | Precomputed bins | 2x faster |
| 🔧 MED | GooglyEye | Multiple sqrt() | ESP-DSP math | 1.5x faster |
| 🔧 MED | Idle Detection | 3x fabsf() | Vector magnitude | Cleaner + faster |
| 🔧 MED | Color Waves | sin() per pixel | Lookup table | 5x faster |
| 🔧 MED | Bitmap Draw | Per-pixel loop | Batch operations | 1.5x faster |
| 💡 LOW | FlyingHeart | Per-heart loop | Batch vectors | Minor |
| 💡 LOW | Smoothing | Basic EMA | ESP-DSP IIR | Minor |

---

## 1. 🎯 HIGH PRIORITY: Replace ArduinoFFT with ESP-DSP in Viseme

**Files:** `src/Renderer/Viseme.cpp`, `src/Renderer/Viseme.h`

### Current Issue
- Uses `ArduinoFFT<double>` which is software-based and operates on `double` (64-bit) floats
- Not utilizing ESP32's hardware-accelerated DSP instructions

### Current Code
```cpp
#include <arduinoFFT.h>

ArduinoFFT<double> FFT = ArduinoFFT<double>(real, imaginary, i2sSamples, i2sSampleRate);
double real[i2sSamples], imaginary[i2sSamples];

void Viseme::calcFFT() {
    FFT.dcRemoval();
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
    FFT.compute(FFTDirection::Forward);
    FFT.complexToMagnitude();
}
```

### Optimized Code
```cpp
#include "esp_dsp.h"

// Use float (32-bit) instead of double (64-bit) - ESP-DSP is optimized for floats
float fftInput[2 * i2sSamples];  // Interleaved complex format [real0, imag0, real1, imag1, ...]

// Initialize once in setup/constructor
void Viseme::initFFT() {
    dsps_fft2r_init_fc32(NULL, i2sSamples);
}

void Viseme::calcFFT() {
    // Apply Hamming window (hardware-accelerated)
    dsps_wind_hann_f32(fftInput, i2sSamples);

    // Perform FFT (hardware-accelerated)
    dsps_fft2r_fc32(fftInput, i2sSamples);

    // Bit reversal
    dsps_bit_rev_fc32(fftInput, i2sSamples);

    // Convert to real spectrum
    dsps_cplx2reC_fc32(fftInput, i2sSamples);
}
```

### Expected Improvement
**3-5x faster** FFT computation. Reference test: `test/fft_performance_comparison.ino`

---

## 2. 🎯 HIGH PRIORITY: Optimize Viseme Frequency Band Analysis

**File:** `src/Renderer/Viseme.cpp` (lines 195-225)

### Current Issue
- Loop calculates frequency for every bin using floating-point division
- Uses multiple `if` statements per iteration
- Redundant calculations every frame

### Current Code
```cpp
for (int i = 4; i < i2sSamples / 2; i++) {
    double freq = i * ((i2sSampleRate / 2.0) / (i2sSamples / 2.0));
    double amplitude = abs(imaginary[i]);
    if (freq >= AHFreqMin && freq <= AHFreqMax) {
        ah_amplitude += amplitude;
    }
    if (freq >= EEFreqMin && freq <= EEFreqMax) {
        ee_amplitude += amplitude;
    }
    // ... more frequency bands
}
```

### Optimized Code
```cpp
// Precompute bin indices at initialization (once)
const float binFreqWidth = i2sSampleRate / (float)i2sSamples;
const int ahStartBin = (int)(AHFreqMin / binFreqWidth);
const int ahEndBin = (int)(AHFreqMax / binFreqWidth);
const int eeStartBin = (int)(EEFreqMin / binFreqWidth);
const int eeEndBin = (int)(EEFreqMax / binFreqWidth);
// ... etc for other bands

// At runtime - direct bin range iteration (no frequency calculation)
for (int i = ahStartBin; i <= ahEndBin; i++) {
    ah_amplitude += sqrtf(fftInput[2*i]*fftInput[2*i] + fftInput[2*i+1]*fftInput[2*i+1]);
}

// Or use ESP-DSP dot product for even faster summing
// dsps_dotprod_f32(&magnitudes[ahStartBin], ones, &ah_amplitude, ahEndBin - ahStartBin);
```

### Expected Improvement
**2x faster** frequency analysis loop

---

## 3. 🔧 MEDIUM PRIORITY: GooglyEye Physics Optimization

**File:** `src/Renderer/GooglyEye.cpp`

### Current Issue
- Multiple `sqrt()` calls per frame (lines 25, 27, 35, 41)
- Multiple floating-point multiplications/divisions
- Called every frame for physics simulation

### Current Code
```cpp
float d = xNew * xNew + yNew * yNew;
float r2 = INNER_RADIUS * INNER_RADIUS;
if (d >= r2) {
    // ... collision detection with multiple sqrt() calls
    float mag1 = sqrt(dx * dx + dy * dy);
    float mag2 = sqrt(dx1 * dx1 + dy1 * dy1);
    // ...
}
```

### Optimized Code
```cpp
#include "esp_dsp.h"

// Option 1: Use fast inverse square root approximation
inline float fastInvSqrt(float x) {
    float xhalf = 0.5f * x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i >> 1);
    x = *(float*)&i;
    return x * (1.5f - xhalf * x * x);
}

// Option 2: Use ESP-DSP sqrt function
float mag1;
dsps_sqrtf_f32_ansi(&input, &mag1, 1);

// Option 3: Avoid sqrt where possible (compare squared distances)
float d_squared = xNew * xNew + yNew * yNew;
if (d_squared >= r2) {  // Compare squared values, avoid sqrt
    // Only compute sqrt when actually needed
}
```

### Expected Improvement
**1.5x faster** physics calculations

---

## 4. 🔧 MEDIUM PRIORITY: Accelerometer Idle Detection

**File:** `src/KMMXController/ControllerIdle.cpp` (lines 36-43)

### Current Issue
- Uses `fabsf()` three times per update cycle
- Separate comparisons for X, Y, Z axes
- Not utilizing vector operations

### Current Code
```cpp
if (fabsf(current.accelX - prev.accelX) < currentThreshold &&
    fabsf(current.accelY - prev.accelY) < currentThreshold &&
    fabsf(current.accelZ - prev.accelZ) < currentThreshold) {
    // Still
}
```

### Optimized Code
```cpp
#include "esp_dsp.h"

// Calculate movement magnitude as single value
float diff[3] = {
    current.accelX - prev.accelX,
    current.accelY - prev.accelY,
    current.accelZ - prev.accelZ
};

// Use dot product to get sum of squares (magnitude squared)
float magnitudeSquared;
dsps_dotprod_f32(diff, diff, &magnitudeSquared, 3);

// Compare squared magnitude to squared threshold (avoid sqrt)
float thresholdSquared = currentThreshold * currentThreshold * 3;  // 3 axes
if (magnitudeSquared < thresholdSquared) {
    // Still
}
```

### Expected Improvement
Cleaner code + slightly faster execution

---

## 5. 🔧 MEDIUM PRIORITY: Color Wave/Pattern Generation

**File:** `src/Devices/LEDMatrixDisplay/Hub75DMA.cpp` (lines 102-120)

### Current Issue
- Uses `sin()` function for wave generation (very expensive)
- Called per-pixel, per-frame
- No caching or lookup tables

### Current Code
```cpp
void Hub75DMA::getColorWave(const uint8_t brightness, const int row, const int col, uint8_t& r, uint8_t& g, uint8_t& b) {
    double wave = (row + col) * 0.4 + millis() * 0.01;
    double color1 = sin(wave) * 127 + 128;
    double color2 = sin(wave + PI) * 127 + 128;
    // ...
}
```

### Optimized Code
```cpp
// Option 1: Precomputed sine lookup table (256 entries)
static const uint8_t sinTable[256] PROGMEM = {
    128, 131, 134, 137, /* ... precomputed values ... */
};

inline uint8_t fastSin(uint8_t angle) {
    return pgm_read_byte(&sinTable[angle]);
}

void Hub75DMA::getColorWave(const uint8_t brightness, const int row, const int col, uint8_t& r, uint8_t& g, uint8_t& b) {
    uint8_t phase = ((row + col) * 102 + (millis() >> 2)) & 0xFF;  // Scale to 0-255
    uint8_t color1 = fastSin(phase);
    uint8_t color2 = fastSin(phase + 128);  // +PI equivalent
    // ...
}

// Option 2: ESP-DSP tone generation for batch processing
// dsps_tone_gen_f32(buffer, length, amplitude, freq, phase);
```

### Expected Improvement
**5x faster** wave generation

---

## 6. 🔧 MEDIUM PRIORITY: Bitmap Drawing Optimization

**File:** `src/Devices/LEDMatrixDisplay/Hub75DMADraw.cpp`

### Current Issue
- Nested loops with per-pixel `pgm_read_byte()` calls
- `getColorMap()` called for every visible pixel
- No batch processing

### Current Code
```cpp
void Hub75DMA::drawBitmap(const uint8_t* bitmap, int imageWidth, int imageHeight, int offsetX, int offsetY) {
    for (int i = 0; i < imageHeight; i++) {
        for (int j = 0; j < imageWidth; j++) {
            uint8_t pixel = pgm_read_byte(bitmap + i * imageWidth + j);
            if (pixel > minimumPixelBrightness) {
                uint8_t r, g, b;
                getColorMap(pixel, i + offsetY, r, g, b);
                matrix->drawPixelRGB888(offsetX + j, offsetY + i, r, g, b);
            }
        }
    }
}
```

### Optimization Ideas
```cpp
// 1. Cache color map for each row (only 32 rows total)
static uint8_t rowColorCache[32][3];  // Precompute at brightness change

// 2. Use memcpy for row-based operations where possible

// 3. Unroll inner loop for common bitmap widths
// 4. Use DMA for bulk pixel transfers if supported by matrix library

// 5. Skip entire rows if they're known to be empty (RLE metadata)
```

### Expected Improvement
**1.5x faster** bitmap rendering

---

## 7. 💡 LOW PRIORITY: FlyingHeart Physics

**File:** `src/Renderer/FlyingHeart.cpp`

### Current Issue
- Multiple random number generations using `esp_random()`
- Loop-based heart movement with per-heart calculations

### Optimization Ideas
```cpp
// Pregenerate random values in batches
static float randomBuffer[64];
static int randomIndex = 0;

void pregenRandom() {
    for (int i = 0; i < 64; i++) {
        randomBuffer[i] = (float)esp_random() / (float)UINT32_MAX;
    }
    randomIndex = 0;
}

float getNextRandom() {
    if (randomIndex >= 64) pregenRandom();
    return randomBuffer[randomIndex++];
}

// Use ESP-DSP vector add for batch position updates
// dsps_add_f32(positions, velocities, positions, numHearts);
```

---

## 8. 💡 LOW PRIORITY: Smoothing Filter Optimization

**Files:** `src/Utils/Utils.cpp`, `src/Renderer/Viseme.cpp`

### Current Issue
- Uses simple exponential smoothing with per-sample calculations

### Current Code
```cpp
smoothedValue = alpha * buffer[i] + (1 - alpha) * smoothedValue;
```

### Optimized Code with ESP-DSP IIR Filter
```cpp
#include "esp_dsp.h"

// Define biquad coefficients for low-pass filter
float coeffs[5] = {b0, b1, b2, a1, a2};  // IIR coefficients
float state[2] = {0, 0};  // Filter state

// Apply hardware-accelerated IIR filter to entire buffer
dsps_biquad_f32(input, output, i2sSamples, coeffs, state);
```

---

## 9. 💡 LOW PRIORITY: Batch Sensor Data Processing

**File:** `src/KMMXController/ControllerSensor.cpp`

### Current Issue
- Processes sensor data one value at a time

### Optimization Ideas
- Buffer multiple sensor readings (e.g., 4-8 samples)
- Process in batches using ESP-DSP vector operations
- Apply filtering to batch data

---

## Implementation Guide

### Step 1: Add ESP-DSP to platformio.ini

```ini
lib_deps =
    espressif/esp-dsp @ ^1.3.4
```

### Step 2: Include ESP-DSP Headers

```cpp
#include "esp_dsp.h"
```

### Step 3: Initialize ESP-DSP (once at startup)

```cpp
void setup() {
    // Initialize FFT tables
    dsps_fft2r_init_fc32(NULL, FFT_SIZE);
}
```

### Step 4: Profile Before/After

```cpp
unsigned long startTime = micros();
// ... operation to measure ...
unsigned long elapsed = micros() - startTime;
Serial.printf("Operation took %lu us\n", elapsed);
```

---

## Reference Test Files

- `test/fft_performance_comparison.ino` - Compares ArduinoFFT vs ESP-DSP performance
- `test/viseme_debug_espdsp.ino` - ESP-DSP implementation of viseme analysis

---

## ESP-DSP Function Reference

| Function | Purpose |
|----------|---------|
| `dsps_fft2r_init_fc32()` | Initialize FFT tables |
| `dsps_fft2r_fc32()` | Perform radix-2 FFT |
| `dsps_bit_rev_fc32()` | Bit reversal after FFT |
| `dsps_cplx2reC_fc32()` | Complex to real conversion |
| `dsps_wind_hann_f32()` | Apply Hann window |
| `dsps_wind_blackman_f32()` | Apply Blackman window |
| `dsps_dotprod_f32()` | Dot product (sum of products) |
| `dsps_add_f32()` | Vector addition |
| `dsps_mul_f32()` | Vector multiplication |
| `dsps_biquad_f32()` | IIR biquad filter |
| `dsps_sqrtf_f32_ansi()` | Square root |

---

## 10. Additional Whole-Project Optimizations (Beyond ESP-DSP)

These improvements are independent of ESP-DSP and can make the whole system more responsive and stable (especially when BLE + rendering + sensors are all active).

### A) Remove blocking delays from the render loop

**File:** `src/KMMXController/ControllerRendering.cpp`

**Current behavior:**
- Uses a blocking delay each frame:

```cpp
delay(1000 / display.getRefreshRate());
```

**Why it matters:**
- This blocks BLE polling and delays other work on the main loop.

**What to do:**
- Replace with a non-blocking frame scheduler (based on `millis()`/`micros()`), or move rendering into its own FreeRTOS task and use `vTaskDelayUntil()`.

---

### B) Make the Viseme task cooperative (avoid starving other tasks)

**File:** `src/FacialStates/MouthState/MouthState.cpp`

**Current behavior:**
- The task loops forever without an explicit yield:

```cpp
while (true) {
    mouthState->visemeFrame = mouthState->viseme.renderViseme();
}
```

**Why it matters:**
- Even if I2S reads block for samples, the FFT work can dominate a core and cause jitter.

**What to do:**
- Add a small `vTaskDelay(1)` or use `vTaskDelayUntil()` to pace the loop.
- Ensure this task priority is lower than your sensor task.
- (Optional, best) Split into two stages: one task reads I2S into a ring buffer, another task performs FFT.

---

### C) Eliminate per-frame STL allocations (heap fragmentation + CPU)

**File:** `src/Renderer/Viseme.cpp`

**Current behavior:**
- `getDominantViseme()` uses `std::map`, `std::vector`, and `std::sort`.

**Why it matters:**
- Dynamic allocation and sorting on every viseme frame is expensive.
- Can fragment heap over long runtimes.

**What to do:**
- Replace with a simple “max + runner-up” selection using fixed variables or a small fixed array.
- When you move to ESP-DSP, also move viseme math to `float` to reduce cost.

---

### D) Remove unnecessary `double` usage across the project

**Files:**
- `src/Renderer/Viseme.h`, `src/Renderer/Viseme.cpp`
- `src/Devices/LEDMatrixDisplay/Hub75DMA.cpp`

**Current behavior:**
- Viseme uses `double` arrays and `ArduinoFFT<double>`.
- Color wave patterns use `double` and `sin()`.

**Why it matters:**
- On ESP32, `double` is significantly slower and increases RAM/flash pressure.

**What to do:**
- Prefer `float` everywhere unless there is a strong reason for `double`.

---

### E) Replace `pow(x, 2)` with `x * x`

**Files:**
- `src/Utils/Utils.cpp` (smoothing curve)
- `src/FacialStates/EyeState/EyeState.cpp` (sleep index)

**Why it matters:**
- `pow()` is much slower than multiplication and increases code size.

**What to do:**
- Convert to multiplication:

```cpp
float y = x * x;
```

---

### F) Replace per-pixel `sin()` waves with a lookup table

**File:** `src/Devices/LEDMatrixDisplay/Hub75DMA.cpp`

**Why it matters:**
- `sin()` per pixel per frame is extremely expensive.

**What to do:**
- Use a 256-entry sine LUT (`uint8_t`) and index it with a phase accumulator.

---

### G) CheekPanel: avoid repainting the whole strip every update

**File:** `src/Devices/Ws2812/CheekPanel.cpp`

**Current behavior:**
- Each animation step repaints background for every pixel, then paints the highlight cluster, then calls `strip.show()`.

**Why it matters:**
- `strip.show()` is time-consuming and can temporarily disable interrupts.
- Writing every pixel every time increases CPU usage.

**What to do:**
- Track the previous highlight positions and only update the pixels that changed.
- Keep the background already written; only clear/update the small highlight set.

---

### H) Make debug logging build-time configurable

**Files:** multiple (`Serial.println`, `Serial.printf`)

**Why it matters:**
- Serial I/O can cause frame jitter and timing instability.

**What to do:**
- Wrap logging with a `#define DEBUG_LOG` macro, or compile it out for release builds.

---

### I) Prefer one-time allocations only (avoid runtime heap churn)

**Files:**
- `src/Network/BLE.cpp` (singleton uses `new` once)
- `src/Devices/LEDMatrixDisplay/Hub75DMA.cpp` (matrix created via `new` once)

**Notes:**
- These one-time allocations are usually fine.
- The main thing to avoid is repeated allocation in hot loops (viseme selection currently does that via STL containers).

---

### J) Optional: move the main loop to a dedicated task

**Files:** `src/main.cpp`, controller code

**Why it matters:**
- You already run a sensor task and a viseme task. Moving rendering to its own paced task can make timing more predictable.

**What to do:**
- Create a rendering task pinned to the opposite core from sensors, pace with `vTaskDelayUntil()`, and keep BLE polling frequent.

---

## Additional Resources

- [ESP-DSP Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/dsp.html)
- [ESP-DSP GitHub](https://github.com/espressif/esp-dsp)
- [ESP32 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)
