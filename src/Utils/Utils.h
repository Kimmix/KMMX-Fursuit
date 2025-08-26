#pragma once

#include <cstddef>  // for size_t
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