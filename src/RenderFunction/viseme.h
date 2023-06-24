#include <map>
#include <arduinoFFT.h>
#include "Devices/I2SMicrophone.h"

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

#define NOISE_THRESHOLD 10000
#define SMOOTHING_ALPHA 0.2  // smoothing factor between 0 and 1

class Viseme {
   public:
    bool microphoneEnable = false;
    const uint8_t* renderViseme() {
        if (!microphoneEnable) {
            microphone.init(SAMPLE_RATE, SAMPLES);
            microphoneEnable = true;
            Serial.println("******** Start Mic ********");
        }
        getDigtalSample(real, imaginary, true);
        // getAnalogSample(real, imaginary, false);
        FFT.DCRemoval();
        FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
        FFT.Compute(FFT_FORWARD);
        FFT.ComplexToMagnitude();
        // double peak = FFT.MajorPeak();
        // Serial.print("xp - ");
        // Serial.println(peak);

        //? Compute amplitude of frequency ranges for each viseme
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
        //? Normalizing
        ah_amplitude *= 0.8;
        ee_amplitude *= 1.0;
        oh_amplitude *= 1.7;
        oo_amplitude *= 1.5;
        th_amplitude *= 2.4;

        VisemeType viseme = AH;
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

        double min_amplitude, max_amplitude, avg_amplitude;
        calculateAmplitude(ah_amplitude, ee_amplitude, oh_amplitude, oo_amplitude, th_amplitude, min_amplitude, max_amplitude, avg_amplitude);
        int loudness_level = calculateLoudness(max_amplitude, avg_amplitude);
        loudness_level = decayLoudness(loudness_level, max_amplitude, min_amplitude);

        //? Debugging
        // int max_threshold = 20000;
        // Serial.print("NOISE_THRESHOLD:");
        // Serial.print(NOISE_THRESHOLD);
        // Serial.print(",Max_THRESHOLD:");
        // Serial.print(max_threshold);
        // Serial.print(",AH:");
        // Serial.print(ah_amplitude > max_threshold ? max_threshold : ah_amplitude);
        // Serial.print(",EE:");
        // Serial.print(ee_amplitude > max_threshold ? max_threshold : ee_amplitude);
        // Serial.print(",OH:");
        // Serial.print(oh_amplitude > max_threshold ? max_threshold : oh_amplitude);
        // Serial.print(",OO:");
        // Serial.print(oo_amplitude > max_threshold ? max_threshold : oo_amplitude);
        // Serial.print(",TH:");
        // Serial.print(th_amplitude > max_threshold ? max_threshold : th_amplitude);
        // Serial.print(",AVG_AMP:");
        // Serial.println(avg_amplitude);

        //! Final render
        return visemeOutput(viseme, loudness_level);
    }

   private:
    I2SMicrophone microphone = I2SMicrophone();
    arduinoFFT FFT = arduinoFFT(real, imaginary, SAMPLES, SAMPLE_RATE);

    enum VisemeType {
        AH,
        EE,
        OH,
        OO,
        TH
    };
    const uint8_t* ahViseme[4] = {AH1, AH2, AH3, AH4};
    const uint8_t* eeViseme[4] = {EE1, EE2, EE3, EE4};
    const uint8_t* ohViseme[4] = {OH1, OH2, OH3, OH4};
    const uint8_t* ooViseme[4] = {OO1, OO2, OO3, OO4};
    const uint8_t* thViseme[4] = {TH1, TH2, TH3, TH4};

    double real[SAMPLES],
        imaginary[SAMPLES];

    const float alpha = SMOOTHING_ALPHA;  // smoothing factor between 0 and 1
    // Read analog microphone input and fill fft_input array with samples
    void getAnalogSample(double* vReal, double* vImagine, bool isSmooth) {
        unsigned int sampling_period_us = round(1000 * (1.0 / SAMPLE_RATE));
        unsigned long microseconds;
        float smoothedValue = 0;
        for (int i = 0; i < SAMPLES; i++) {
            microseconds = millis();
            if (isSmooth) {
                smoothedValue = alpha * analogRead(13) + (1 - alpha) * smoothedValue;  // apply exponential smoothing
                vReal[i] = smoothedValue;
            } else {
                vReal[i] = analogRead(13);
            }
            vImagine[i] = 0;
            while (millis() < (microseconds + sampling_period_us)) {
            }
        }
    };

    void getDigtalSample(double* vReal, double* vImagine, bool isSmooth) {
        int16_t buffer[SAMPLES];
        microphone.read(buffer, SAMPLES);
        float smoothedValue = 0;
        for (int i = 0; i < SAMPLES; i++) {
            if (isSmooth) {
                smoothedValue = alpha * buffer[i] + (1 - alpha) * smoothedValue;  // apply exponential smoothing
                vReal[i] = smoothedValue;
            } else {
                vReal[i] = buffer[i];
            }
            vImagine[i] = 0;
        }
    }

    void calculateAmplitude(double ah, double ee, double oh, double oo, double th, double& minAmp, double& maxAmp, double& avgAmp) {
        minAmp = min(min(min(min(ah, ee), oh), oo), th);
        maxAmp = max(max(max(max(ah, ee), oh), oo), th);
        avgAmp = (ah + ee + oh + oo + th) / 5.0;
    }
    // Compute loudness level based on average amplitude
    int calculateLoudness(double max, double avg) {
        if (max > avg * 2.2) {
            return 4;
        } else if (max > avg * 1.8) {
            return 3;
        } else if (max > avg * 1.6) {
            return 2;
        } else if (max > avg * 1.4) {
            return 1;
        }
        return 0;
    }

    VisemeType previousViseme;
    VisemeType holdViseme(VisemeType input) {
        if (input != 0) {
            return input;
        }
        previousViseme = input;
        return previousViseme;
    }
    int previousLoudness = 0;                          // Initialize previous input variable to 0
    unsigned long decayStartTime = 0;                  // Initialize decay start time to 0
    const double decayRate = 0.75;                     // Set the decay rate to 0.75
    const unsigned long decayElapsedThreshold = 1000;  // Set the decay elapsed time threshold to 1 seconds
    int decayLoudness(int input, double max_amplitude, double min_amplitude) {
        if (max_amplitude > NOISE_THRESHOLD || previousLoudness > 0) {
            if (input >= previousLoudness) {                                                             // If the new input is greater than or equal to the previous input
                previousLoudness = input;                                                                // Update the previous input to the new input
                decayStartTime = 0;                                                                      // Reset the decay start time
            } else {                                                                                     // If the new input is less than the previous input
                if (decayStartTime == 0) {                                                               // If this is the first time the input has decayed
                    decayStartTime = millis();                                                           // Set the decay start time to the current time
                } else {                                                                                 // If the input is currently decaying
                    unsigned long decay_elapsed_time = millis() - decayStartTime;                        // Calculate the elapsed decay time
                    if (decay_elapsed_time >= decayElapsedThreshold) {                                   // If the decay elapsed time exceeds the threshold
                        previousLoudness = 0;                                                        // Update the previous input to the new input
                        decayStartTime = 0;                                                              // Reset the decay start time
                    } else {                                                                             // If the input is still decaying
                        int decayed_input = previousLoudness - (decayRate * decay_elapsed_time / 1000);  // Calculate the decayed input
                        if (decayed_input < input) {                                                     // If the decayed input is less than the new input
                            previousLoudness = input;                                                    // Update the previous input to the new input
                            decayStartTime = 0;                                                          // Reset the decay start time
                        } else {                                                                         // If the decayed input is greater than or equal to the new input
                            previousLoudness = decayed_input;                                            // Update the previous input to the decayed input
                        }
                    }
                }
            }
            return previousLoudness;  // Return the current input value
        }
        return 0;
    }

    const uint8_t* visemeOutput(VisemeType viseme, int level) {
        if (level == 0) {
            return mouthDefault;
        }
        level -= 1;
        Serial.print("Base:");
        Serial.print(4);
        Serial.print(",Level:");
        Serial.println(level);
        Serial.print("viseme:");
        Serial.print(viseme);
        Serial.print(",previousViseme:");
        Serial.print(previousViseme);
        auto combination = visemeCombination.find(std::make_pair(viseme, previousViseme));
        Serial.print(",combination:");
        Serial.println(combination->second);
        previousViseme = viseme;
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
        return NULL;
    };

    std::map<std::pair<int, int>, String> visemeCombination = {
        {{AH, AH}, "AHViseme"},
        {{EE, EE}, "EEViseme"},
        {{OH, OH}, "OHViseme"},
        {{OO, OO}, "OOViseme"},
        {{TH, TH}, "THViseme"},
        {{AH, EE}, "AHEEViseme"},
        {{AH, OH}, "AHOHViseme"},
        {{AH, OO}, "AHOOViseme"},
        {{AH, TH}, "AHTHViseme"},
        {{EE, AH}, "AHEEViseme"},
        {{EE, OH}, "EEOHViseme"},
        {{EE, OO}, "EEOOViseme"},
        {{EE, TH}, "EETHViseme"},
        {{OH, AH}, "AHOHViseme"},
        {{OH, EE}, "EEOHViseme"},
        {{OH, OO}, "OHOOViseme"},
        {{OH, TH}, "OHTHViseme"},
        {{OO, AH}, "AHOOViseme"},
        {{OO, EE}, "EEOOViseme"},
        {{OO, OH}, "OHOOViseme"},
        {{OO, TH}, "OOTHViseme"},
        {{TH, AH}, "AHTHViseme"},
        {{TH, EE}, "EETHViseme"},
        {{TH, OH}, "OHTHViseme"},
        {{TH, OO}, "OOTHViseme"}};
};
