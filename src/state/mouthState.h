#include <Arduino.h>
#include <arduinoFFT.h>
#include <driver/i2s.h>
#include "mouthBitmap.h"

#define AH_MIN 600
#define AH_MAX 1200
#define EE_MIN 1000
#define EE_MAX 2000
#define OH_MIN 1800
#define OH_MAX 2800
#define OO_MIN 2600
#define OO_MAX 3600
#define TH_MIN 3400
#define TH_MAX 5000

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
            display->drawMouth(mouthOpen);
            currentState = TALKING;
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
    int levelofLoudness = 3;
    arduinoFFT FFT = arduinoFFT(fft_input, fft_output, SAMPLES, SAMPLE_RATE);


    enum State {
        IDLE,
        BOOP,
        TALKING,
    };
    State currentState;

    enum Viseme {
        AH,
        EE,
        OH,
        OO,
        TH
    };
    // ---- Viseme ----
    const uint8_t* ahViseme[3] = { AH1, AH2, AH3 };
    const uint8_t* eeViseme[3] = { EE1, EE2, EE3 };
    const uint8_t* ohViseme[3] = { OH1, OH2, OH3 };
    const uint8_t* ooViseme[3] = { OO1, OO2, OO3 };
    const uint8_t* thViseme[3] = { TH1, TH2, TH3 };

    const uint8_t* visemeOutput(Viseme viseme, int level) {
        switch (viseme) {
        case AH:
            return ahViseme[level];
        case EE:
            return eeViseme[level];
        case OH:
            return ohViseme[level];
        case OO:
            return ooViseme[level];
        case TH:
            return thViseme[level];
        };
    };

    unsigned long microseconds;
    unsigned long sampling_period_us;

    // smoothing factor between 0 and 1
    const float alpha = 0.2;
    float smoothedValue = 0;

    void talking() {
        double max_amplitude = 0;
        double min_amplitude = 0;
        // Read microphone input and fill fft_input array with samples
        // int16_t buffer[SAMPLES];
        // mic->read(buffer, SAMPLES);
        for (int i = 0; i < SAMPLES; i++) {
            microseconds = millis();

            // apply exponential smoothing
            smoothedValue = alpha * analogRead(39) + (1 - alpha) * smoothedValue;
            fft_input[i] = smoothedValue;
            fft_output[i] = 0;

            while (millis() < (microseconds + sampling_period_us)) {}
        }
        // FFT.DCRemoval();
        FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
        FFT.Compute(FFT_FORWARD);
        FFT.ComplexToMagnitude();
        double f, v;
        FFT.MajorPeak(&f, &v);
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
            // if (amplitude > max_amplitude) {
            //     max_amplitude = amplitude;
            // }
        }
        // Normalizing
        ah_amplitude *= 0.4;
        ee_amplitude *= 0.5;
        oh_amplitude *= 1.2;
        oo_amplitude *= 2.0;
        th_amplitude *= 1.6;

        max_amplitude = max(max(max(max(ah_amplitude, ee_amplitude), oh_amplitude), oo_amplitude), th_amplitude);
        min_amplitude = min(min(min(min(ah_amplitude, ee_amplitude), oh_amplitude), oo_amplitude), th_amplitude);

        // Compute loudness level based on average amplitude
        double avg_amplitude = (ah_amplitude + ee_amplitude + oh_amplitude + oo_amplitude + th_amplitude) / 5.0;
        int loudness_level = 0;
        if (avg_amplitude > max_amplitude * 2) {
            loudness_level = 2;
        }
        else if (avg_amplitude > max_amplitude) {
            loudness_level = 1;
        }

        // Determine which viseme to display based on amplitude and threshold values
        Viseme viseme = AH;
        double viseme_amplitude = ah_amplitude;
        if (ee_amplitude > viseme_amplitude) {
            viseme = EE;
            viseme_amplitude = ee_amplitude;
        }
        if (oh_amplitude > viseme_amplitude) {
            viseme = OH;
            viseme_amplitude = oh_amplitude;
        }
        if (oo_amplitude > viseme_amplitude) {
            viseme = OO;
            viseme_amplitude = oo_amplitude;
        }
        if (th_amplitude > viseme_amplitude) {
            viseme = TH;
        }

        Serial.print("AH:");
        Serial.print(ah_amplitude);
        Serial.print(",");
        Serial.print("EE:");
        Serial.print(ee_amplitude);
        Serial.print(",");
        Serial.print("OH:");
        Serial.print(oh_amplitude);
        Serial.print(",");
        Serial.print("OO:");
        Serial.print(oo_amplitude);
        Serial.print(",");
        Serial.print("TH:");
        Serial.print(th_amplitude);
        Serial.print(",");
        Serial.print("AVG_AMP:");
        Serial.println(avg_amplitude);
        // Serial.print(",");
        // Serial.print("threshold:");
        // Serial.println(max_amplitude);

        // Print results
        // Serial.print("Viseme: ");
        // Serial.print(viseme);
        // Serial.print("| Loudness:");
        // Serial.print(loudness_level);
        // Serial.println(" dB");

        // Final render
        if (max_amplitude - min_amplitude > 2000) {
            display->drawMouth(visemeOutput(viseme, loudness_level));
        }
        else {
            display->drawMouth(mouthDefault);
        }
    }
};