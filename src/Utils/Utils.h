#pragma once

#include <cstddef>  // for size_t
#include <stdint.h>
#include <string.h>  // for memcpy

/**
 * @brief Fast clamp/constrain function optimized for ESP32.
 *
 * Replaces Arduino's constrain() with faster inline implementation.
 * Uses simple if-statements which are faster than the macro version.
 *
 * @tparam T The type of value to clamp
 * @param value The value to clamp
 * @param minVal The minimum value
 * @param maxVal The maximum value
 * @return Clamped value
 */
template <typename T>
inline T fastClamp(T value, T minVal, T maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

/**
 * @brief Fast integer map function optimized for ESP32.
 *
 * Replaces Arduino's map() with optimized inline computation.
 * Uses 32-bit math to prevent overflow and clamps result to output range.
 *
 * @param x Input value
 * @param inMin Input range minimum
 * @param inMax Input range maximum
 * @param outMin Output range minimum
 * @param outMax Output range maximum
 * @return Mapped and clamped value
 */
template <typename T>
inline T fastMap(T x, T inMin, T inMax, T outMin, T outMax) {
    // Pre-calculate ranges to avoid redundant subtraction
    T inRange = inMax - inMin;
    T outRange = outMax - outMin;

    // Compute mapped value: (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin
    T result = ((x - inMin) * outRange) / inRange + outMin;

    // Clamp to output range
    return fastClamp(result, outMin, outMax);
}

/**
 * @brief Maps a float from one range to another and constrains the result.
 *
 * This function takes an input float `x` and maps it from the input range
 * [inMin, inMax] to the output range [outMin, outMax]. The result is
 * constrained to stay within the output range.
 *
 * @param x The input value.
 * @param inMin The minimum of the input range.
 * @param inMax The maximum of the input range.
 * @param outMin The minimum of the output range.
 * @param outMax The maximum of the output range.
 * @return The mapped and constrained value.
 */
float mapFloat(float x, float inMin, float inMax, float outMin, float outMax);

/**
 * @brief Smooths accelerometer data and calculates animation levels with natural movement.
 *
 * This function applies smoothing, dead zones, and easing curves to raw accelerometer
 * data to create more natural and fluid animations for facial expressions.
 *
 * @param rawValue The raw accelerometer reading.
 * @param smoothedValue Reference to the smoothed value (maintains state between calls).
 * @param threshold The activation threshold.
 * @param maxThreshold The maximum threshold for full animation.
 * @param smoothingFactor The smoothing factor (0.0-1.0, lower = smoother).
 * @param deadZone The dead zone around the threshold to prevent micro-movements.
 * @param maxLevel The maximum animation level (e.g., 19 for 20 frames).
 * @param isNegativeDirection True if checking for negative direction movement.
 * @return The calculated animation level, or -1 if no movement detected.
 */
int smoothAccelerometerMovement(float rawValue, float& smoothedValue,
                               float threshold, float maxThreshold,
                               float smoothingFactor = 0.3f, float deadZone = 0.5f,
                               int maxLevel = 19, bool isNegativeDirection = false);

/**
 * @brief Tracks and displays FPS in Serial output.
 *
 * Call this function once per frame to track frame rate.
 * Prints FPS to Serial every second.
 */
void showFPS();

/**
 * @brief Generic median filter using insertion sort.
 *
 * Applies median filter to an array of values for noise rejection.
 * Uses insertion sort (O(n²) worst case but O(n) best case for nearly-sorted data).
 * For small arrays (< 10 elements), this is faster than quicksort and doesn't require ESP-DSP.
 *
 * Median filter is more effective than averaging for removing outliers/spikes.
 * Uses memcpy for DMA-optimized copying on ESP32.
 *
 * @tparam T The type of array elements (must support comparison operators)
 * @tparam N The size of the array
 * @param buffer The input buffer array
 * @param count The number of valid elements to process (can be less than N)
 * @return The median value
 */
template <typename T, size_t N>
T medianFilter(const T* buffer, size_t count) {
    if (count == 0) return T(0);
    if (count > N) count = N;

    // Create a copy of buffer for sorting (don't modify original)
    T sorted[N];
    memcpy(sorted, buffer, count * sizeof(T));

    // Insertion sort - optimized for small, nearly-sorted arrays
    for (size_t i = 1; i < count; i++) {
        T key = sorted[i];
        int j = i - 1;

        // Shift elements greater than key to the right
        while (j >= 0 && sorted[j] > key) {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = key;
    }

    // Return median value (middle element)
    return sorted[count / 2];
}

/**
 * @brief Fast exponential smoothing filter (single-pole IIR low-pass filter).
 *
 * Commonly used pattern for sensor smoothing throughout the codebase.
 * Formula: output = alpha * input + (1 - alpha) * previousOutput
 *
 * @tparam T The type of values to smooth (usually float)
 * @param input The new input value
 * @param previousOutput The previous smoothed output (updated in-place)
 * @param alpha Smoothing factor (0.0 = no change, 1.0 = no smoothing)
 * @return The new smoothed value
 */
template <typename T>
inline T exponentialSmoothing(T input, T& previousOutput, float alpha) {
    previousOutput = (alpha * input) + ((1.0f - alpha) * previousOutput);
    return previousOutput;
}

/**
 * @brief Linear interpolation (lerp) between two values.
 *
 * @tparam T The type of values to interpolate
 * @param start The starting value
 * @param end The ending value
 * @param t Interpolation factor (0.0 = start, 1.0 = end)
 * @return The interpolated value
 */
template <typename T>
inline T lerp(T start, T end, float t) {
    return start + t * (end - start);
}

/**
 * @brief Inverse linear interpolation (inverse lerp).
 *
 * Calculates the interpolation factor needed to reach a value.
 *
 * @tparam T The type of values
 * @param start The starting value
 * @param end The ending value
 * @param value The current value
 * @return The interpolation factor (0.0 to 1.0)
 */
template <typename T>
inline float inverseLerp(T start, T end, T value) {
    if (start == end) return 0.0f;
    return (float)(value - start) / (float)(end - start);
}

/**
 * @brief Ease-out curve (decelerating interpolation).
 *
 * Uses quadratic easing: f(t) = 1 - (1-t)²
 * Already used in smoothAccelerometerMovement, now available globally.
 *
 * @param t Input value (typically 0.0 to 1.0)
 * @return Eased value
 */
inline float easeOut(float t) {
    float complement = 1.0f - t;
    return 1.0f - (complement * complement);
}

/**
 * @brief Ease-in curve (accelerating interpolation).
 *
 * Uses quadratic easing: f(t) = t²
 *
 * @param t Input value (typically 0.0 to 1.0)
 * @return Eased value
 */
inline float easeIn(float t) {
    return t * t;
}

/**
 * @brief Ease-in-out curve (accelerating then decelerating).
 *
 * Uses smooth cubic easing for natural motion.
 *
 * @param t Input value (typically 0.0 to 1.0)
 * @return Eased value
 */
inline float easeInOut(float t) {
    if (t < 0.5f) {
        return 2.0f * t * t;
    } else {
        float complement = 1.0f - t;
        return 1.0f - 2.0f * complement * complement;
    }
}

/**
 * @brief Returns the length of a fixed-size array.
 *
 * This function template returns the number of elements in a
 * fixed-size array. It works for any type and size of array.
 *
 * @tparam T The type of the array elements.
 * @tparam N The number of elements in the array.
 * @param array The array whose length is to be determined.
 * @return The number of elements in the array.
 */
template <typename T, size_t N>
constexpr size_t arrayLength(T (&)[N]) {
    return N;
}