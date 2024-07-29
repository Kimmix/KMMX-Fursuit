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