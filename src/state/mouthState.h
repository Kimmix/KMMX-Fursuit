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

#define AH_THRESH 1000
#define EE_THRESH 1500
#define OH_THRESH 2000
#define OO_THRESH 2500
#define TH_THRESH 3000

#define SAMPLE_RATE 8000
#define NUM_SAMPLES 256

class MouthState {
public:
    MouthState(DisplayController* displayPtr = nullptr, Microphone* micPtr = nullptr):
        display(displayPtr),
        mic(micPtr),
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
    Microphone* mic;
    arduinoFFT FFT = arduinoFFT();

    double fft_input[NUM_SAMPLES],
        fft_output[NUM_SAMPLES];
    int loudness_thresholds[4] = { 1000, 2000, 3000, 4000 };

    enum State {
        IDLE,
        BOOP,
        TALKING,
    };

    State currentState;

    void talking() {
        // Read microphone input and fill fft_input array with samples
        int16_t buffer[NUM_SAMPLES];
        mic->read(buffer, NUM_SAMPLES);
        for (int16_t i = 0; i < NUM_SAMPLES; ++i) {
            Serial.print(buffer[i]);
            Serial.print("\t");
        }
        Serial.println();

        FFT.Windowing(fft_input, NUM_SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
        FFT.Compute(fft_input, fft_output, NUM_SAMPLES, FFT_FORWARD);
        FFT.ComplexToMagnitude(fft_input, fft_output, NUM_SAMPLES);

        // Compute amplitude of frequency ranges for each viseme
        double ah_amplitude = 0;
        double ee_amplitude = 0;
        double oh_amplitude = 0;
        double oo_amplitude = 0;
        double th_amplitude = 0;

        for (int i = 0; i < NUM_SAMPLES / 2; i++) {
            double freq = i * ((SAMPLE_RATE / 2.0) / (NUM_SAMPLES / 2.0));
            double amplitude = fft_output[i];
            Serial.println(amplitude);
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
        String viseme = "none";
        if (ah_amplitude > AH_THRESH) {
            viseme = "ah";
        }
        else if (ee_amplitude > EE_THRESH) {
            viseme = "ee";
        }
        else if (oh_amplitude > OH_THRESH) {
            viseme = "oh";
        }
        else if (oo_amplitude > OO_THRESH) {
            viseme = "oo";
        }
        else if (th_amplitude > TH_THRESH) {
            viseme = "th";
        }

        // Compute loudness level based on average amplitude
        double avg_amplitude = (ah_amplitude + ee_amplitude + oh_amplitude + oo_amplitude + th_amplitude) / 5.0;
        int loudness_level = 0;
        for (int i = 0; i < 4; i++) {
            if (avg_amplitude > loudness_thresholds[i]) {
                loudness_level = i + 1;
            }
        }

        // Print results
        // Serial.print("Viseme: ");
        // Serial.print(viseme);
        // Serial.print(" | Loudness: ");
        // Serial.print(loudness_level);
        // Serial.println(" dB");
    }
    void test() {
        int16_t buffer[NUM_SAMPLES];
        size_t bytes_read;
        esp_err_t result = i2s_read(I2S_PORT, &buffer, NUM_SAMPLES * 2, &bytes_read, portMAX_DELAY);

        if (result == ESP_OK)
        {
            // Read I2S data buffer
            int16_t samples_read = bytes_read / 8;
            if (samples_read > 0) {
                float mean = 0;
                for (int16_t i = 0; i < samples_read; ++i) {
                    mean += (buffer[i]);
                }

                // Average the data reading
                mean /= samples_read;

                // Print to serial plotter
                Serial.println(mean);
            }
        }
    }
};