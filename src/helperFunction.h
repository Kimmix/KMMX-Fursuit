uint16_t fps = 0;
unsigned long fps_timer;
void showFPS() {
    ++fps;
    if (millis() > fps_timer) {
        Serial.printf_P(PSTR("Effect fps: %d\n"), fps);
        fps_timer = millis() + 1000;
        fps = 0;
    }
}

float mapFloat(float x, float inMin, float inMax, float outMin, float outMax) {
    int mappedValue = int((x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin);
    return constrain(mappedValue, outMin, outMax);
}