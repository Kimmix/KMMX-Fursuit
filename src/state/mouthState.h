#include <Arduino.h>
#include <arduinoFFT.h>
#include <driver/i2s.h>

#define AH_MIN 600
#define AH_MAX 1200
#define EE_MIN 1000
#define EE_MAX 2000
#define OH_MIN 1800
#define OH_MAX 2800
#define OO_MIN 2600
#define OO_MAX 3600
#define TH_MIN 3400
#define TH_MAX 4400

#define SAMPLE_RATE 8000
#define SAMPLES 256

class MouthState {
public:
    MouthState(DisplayController* displayPtr = nullptr):
        display(displayPtr),
        currentState(IDLE)
    {}

    void update() {
        // Serial.print(currentState);
        // Serial.print("\n");
        switch (currentState) {
        case IDLE:
            display->drawMouth(mouthDefault);
            break;
        case BOOP:
            display->drawMouth(mouthAh);
            currentState = IDLE;
            break;
        case TALKING:
            talking();
            break;
        default:
            break;
        }
    }

    void setIdle() {
        currentState = IDLE;
    }
    void setTalk() {
        currentState = TALKING;
    }
    void setBoop() {
        currentState = BOOP;
    }

private:
    DisplayController* display;

    double fft_input[SAMPLES],
        fft_output[SAMPLES],
        loudness_thresholds[4] = { 500, 1000, 1500, 2000 };
    arduinoFFT FFT = arduinoFFT(fft_input, fft_output, SAMPLES, SAMPLE_RATE);


    enum State {
        IDLE,
        BOOP,
        TALKING,
    };

    State currentState;
    unsigned long microseconds;
    unsigned long sampling_period_us;
    void talking() {
        // Read microphone input and fill fft_input array with samples
        // int16_t buffer[SAMPLES];
        // mic->read(buffer, SAMPLES);
        for (int i = 0; i < SAMPLES; i++) {
            microseconds = millis();
            fft_input[i] = analogRead(39);
            fft_output[i] = 0;

            while (millis() < (microseconds + sampling_period_us)) {}
        }
        // FFT.DCRemoval();
        FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
        FFT.Compute(FFT_FORWARD);
        FFT.ComplexToMagnitude();
        // double f, v;
        // FFT.MajorPeak(&f, &v);
        // Serial.println(); Serial.print((int)(f / SAMPLE_RATE * 32 * 2)); Serial.print(" ");
        // Serial.print(f, 6); Serial.print(", "); Serial.println(v, 6);Serial.println("\n\r");

        // Compute amplitude of frequency ranges for each viseme
        double ah_amplitude = 0, ee_amplitude = 0,
            oh_amplitude = 0, oo_amplitude = 0, th_amplitude = 0;

        for (int i = 0; i < SAMPLES / 2; i++) {
            double freq = i * ((SAMPLE_RATE / 2.0) / (SAMPLES / 2.0));
            double amplitude = abs(fft_output[i]);
            if (freq >= AH_MIN && freq <= AH_MAX) {
                ah_amplitude += amplitude;
            }
            if (freq >= EE_MIN && freq <= EE_MAX) {
                ee_amplitude += amplitude;
            }
            if (freq >= OH_MIN && freq <= OH_MAX) {
                oh_amplitude += amplitude;
            }
            if (freq >= OO_MIN && freq <= OO_MAX) {
                oo_amplitude += amplitude;
            }
            if (freq >= TH_MIN && freq <= TH_MAX) {
                th_amplitude += amplitude;
            }
        }

        // Determine which viseme to display based on amplitude and threshold values
        String viseme = "ah";
        double viseme_amplitude = ah_amplitude;
        if (ee_amplitude > viseme_amplitude) {
            viseme = "ee";
            viseme_amplitude = ee_amplitude;
        }
        if (oh_amplitude > viseme_amplitude) {
            viseme = "oh";
            viseme_amplitude = oh_amplitude;
        }
        if (oo_amplitude > viseme_amplitude) {
            viseme = "oo";
            viseme_amplitude = oo_amplitude;
        }
        if (th_amplitude > viseme_amplitude) {
            viseme = "th";
        }

        // Compute loudness level based on average amplitude
        double avg_amplitude = abs(ah_amplitude + ee_amplitude + oh_amplitude + oo_amplitude + th_amplitude) / 5.0;
        int loudness_level = 0;
        for (int i = 0; i < 4; i++) {
            if (avg_amplitude > loudness_thresholds[i]) {
                loudness_level = i + 1;
            }
        }
        // Serial.print("AH:");
        // Serial.print(ah_amplitude);
        // Serial.print(",");
        // Serial.print("EE:");
        // Serial.print(ee_amplitude);
        // Serial.print(",");
        // Serial.print("OH:");
        // Serial.print(oh_amplitude);
        // Serial.print(",");
        // Serial.print("OO:");
        // Serial.print(oo_amplitude);
        // Serial.print(",");
        // Serial.print("TH:");
        // Serial.print(th_amplitude);
        // Serial.print(",");
        // Serial.print("AVG_AMP:");
        // Serial.println(avg_amplitude);
        // Serial.print(",");

        // Print results
        Serial.print("Viseme: ");
        Serial.print(viseme);
        Serial.print("| Loudness:");
        Serial.print(loudness_level);
        Serial.println(" dB");
    }
};