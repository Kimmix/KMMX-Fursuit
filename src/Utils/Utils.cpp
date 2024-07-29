#include "Utils.h"
#include <Arduino.h>

float mapFloat(float x, float inMin, float inMax, float outMin, float outMax) {
    float mappedValue = (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
    return constrain(mappedValue, outMin, outMax);
}
