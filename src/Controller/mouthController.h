#include <arduinoFFT.h>
#include "Bitmaps/mouthBitmap.h"

#define AH_MIN 600
#define AH_MAX 1200
#define EE_MIN 1000
#define EE_MAX 2000
#define OH_MIN 1800
#define OH_MAX 2800
#define OO_MIN 2600
#define OO_MAX 3600
#define TH_MIN 3400
#define TH_MAX 4000

#define SAMPLE_RATE 8000
#define SAMPLES 256

class MouthState {
public:
    MouthState(DisplayController* displayPtr = nullptr, Microphone* microphonePtr = nullptr):
        display(displayPtr),
        microphone(microphonePtr),
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
    Microphone* microphone;
    arduinoFFT FFT = arduinoFFT(real, imaginary, SAMPLES, SAMPLE_RATE);

    enum State {
        IDLE,
        BOOP,
        TALKING,
    };
    State currentState;

    // ---- Viseme ----
    enum Viseme {
        AH,
        EE,
        OH,
        OO,
        TH
    };
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

    double real[SAMPLES],
        imaginary[SAMPLES];
    unsigned long microseconds;
    unsigned int sampling_period_us = round(1000 * (1.0 / SAMPLE_RATE));
    
    const float alpha = 0.2; // smoothing factor between 0 and 1
    float smoothedValue = 0;

    void talking() {
        // Read analog microphone input and fill fft_input array with samples
        // int16_t buffer[SAMPLES];
        // microphone->read(buffer, SAMPLES);
        // for (int i = 0; i < SAMPLES; i++) {
        //     real[i] = buffer[i] / 32768.0;
        //     imaginary[i] = 0;
        // }

        for (int i = 0; i < SAMPLES; i++) {
            microseconds = millis();

            // apply exponential smoothing
            smoothedValue = alpha * analogRead(13) + (1 - alpha) * smoothedValue;
            real[i] = smoothedValue;
            imaginary[i] = 0;

            while (millis() < (microseconds + sampling_period_us)) {}
        }

        FFT.DCRemoval();
        FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
        FFT.Compute(FFT_FORWARD);
        FFT.ComplexToMagnitude();
        // double peak = FFT.MajorPeak();
        // Serial.print("xp - ");
        // Serial.println(peak);

        // Compute amplitude of frequency ranges for each viseme
        double ah_amplitude = 0, ee_amplitude = 0,
            oh_amplitude = 0, oo_amplitude = 0, th_amplitude = 0;

        for (int i = 4; i < SAMPLES / 2; i++) {
            double freq = i * ((SAMPLE_RATE / 2.0) / (SAMPLES / 2.0));
            double amplitude = abs(imaginary[i]);
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
        // Normalizing
        // ah_amplitude *= 0.5;
        // ee_amplitude *= 0.6;
        // oh_amplitude *= 1.8;
        // oo_amplitude *= 2.0;
        // th_amplitude *= 2.3;

        // Compute loudness level based on average amplitude
        double max_amplitude = 0, min_amplitude = 0;
        max_amplitude = max(max(max(max(ah_amplitude, ee_amplitude), oh_amplitude), oo_amplitude), th_amplitude);
        min_amplitude = min(min(min(min(ah_amplitude, ee_amplitude), oh_amplitude), oo_amplitude), th_amplitude);
        double avg_amplitude = (ah_amplitude + ee_amplitude + oh_amplitude + oo_amplitude + th_amplitude) / 5.0;
        int loudness_level = 0;
        if (max_amplitude > avg_amplitude * 2) {
            loudness_level = 2;
        }
        else if (max_amplitude > avg_amplitude * 1.5) {
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

        // Serial.print("AH:");
        // Serial.print(ah_amplitude);
        // Serial.print(",EE:");
        // Serial.print(ee_amplitude);
        // Serial.print(",OH:");
        // Serial.print(oh_amplitude);
        // Serial.print(",OO:");
        // Serial.print(oo_amplitude);
        // Serial.print(",TH:");
        // Serial.println(th_amplitude);
        // Serial.print(",AVG_AMP:");
        // Serial.print(avg_amplitude);
        // Serial.print(",MAX_AMP:");
        // Serial.println(max_amplitude);

        // Print results
        // Serial.print("Viseme:");
        // Serial.print(viseme);
        // Serial.print(",Loudness:");
        // Serial.println(loudness_level);

        // Final render
        if (max_amplitude - min_amplitude > 2000) {
            display->drawMouth(visemeOutput(holdViseme(viseme), decayLoudness(loudness_level)));
        }
        else {
            display->drawMouth(mouthDefault);
        }
    }

    int previous_input = 0;    // Initialize previous input variable to 0
    unsigned long decay_start_time = 0;   // Initialize decay start time to 0
    const double decay_rate = 0.5;   // Set the decay rate (adjust as needed)
    int decayLoudness(int input) {
        if (input >= previous_input) {   // If the new input is greater than or equal to the previous input
            previous_input = input;    // Update the previous input to the new input
            decay_start_time = 0;   // Reset the decay start time
        }
        else {    // If the new input is less than the previous input
            if (decay_start_time == 0) {   // If this is the first time the input has decayed
                decay_start_time = millis();   // Set the decay start time to the current time
            }
            else {    // If the input is currently decaying
                unsigned long decay_elapsed_time = millis() - decay_start_time;   // Calculate the elapsed decay time
                int decayed_input = previous_input - (decay_rate * decay_elapsed_time / 1000);   // Calculate the decayed input
                if (decayed_input < input) {   // If the decayed input is less than the new input
                    previous_input = input;    // Update the previous input to the new input
                    decay_start_time = 0;   // Reset the decay start time
                }
                else {    // If the decayed input is greater than or equal to the new input
                    previous_input = decayed_input;    // Update the previous input to the decayed input
                }
            }
        }
        return previous_input;   // Return the current input value
    }
    Viseme lastViseme;
    Viseme holdViseme(Viseme input) {
        if (input != 0) {
            return input;
        }
        lastViseme = input;
        return lastViseme;
    }
};