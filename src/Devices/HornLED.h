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

    void setBrightness(unsigned short value) {
        const unsigned short minBrightness = 50;
        const unsigned short maxBrightness = 200;

        if (value > maxBrightness) {
            value = maxBrightness;
        } else if (value < minBrightness) {
            value = minBrightness;
        }

        brightness = value;
        ledcWrite(pwmChannel, brightness);
    }

    unsigned short getBrightness() const {
        return brightness;
    }
};
