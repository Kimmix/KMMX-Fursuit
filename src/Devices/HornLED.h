class HornLED {
   private:
    unsigned short brightness = 100,
                   frequency = 20000,
                   pwmChannel = 0,
                   resolution = 8;

   public:
    HornLED(int ledPin = LED_PWM) {
        ledcSetup(pwmChannel, frequency, resolution);
        ledcAttachPin(ledPin, pwmChannel);
        ledcWrite(pwmChannel, brightness);
    }

    const unsigned short minBrightness = 50;
    const unsigned short maxBrightness = 180;
    void setBrightness(unsigned short value) {
        brightness = map(value, 0, 100, minBrightness, maxBrightness);
        ledcWrite(pwmChannel, brightness);
    }

    unsigned short getBrightness() const {
        return brightness;
    }
};
