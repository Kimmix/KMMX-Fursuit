#include <map>
#include <arduinoFFT.h>
#include "Devices/I2SMicrophone.h"
#include "Bitmaps/mouthViseme.h"

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

#define NOISE_THRESHOLD 400
#define SMOOTHING_ALPHA 0.2  // smoothing factor between 0 and 1

class Viseme {
   public:
    void initMic() {
        microphone.init(SAMPLE_RATE, SAMPLES);
    }
    const uint8_t* renderViseme() {
        getDigtalSample(real, imaginary, true);
        // getAnalogSample(real, imaginary, false);
        FFT.DCRemoval();
        FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
        FFT.Compute(FFT_FORWARD);
        FFT.ComplexToMagnitude();

        //? Compute amplitude of frequency ranges for each viseme
        double ah_amplitude = 0, ee_amplitude = 0,
               oh_amplitude = 0, oo_amplitude = 0, th_amplitude = 0;
        double min_amplitude, max_amplitude, avg_amplitude;
        unsigned int loudness_level;

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
        normalizeViseme(ah_amplitude, ee_amplitude, oh_amplitude, oo_amplitude, th_amplitude);
        // Find viseme
        VisemeType dominantViseme = getDominantViseme(ah_amplitude, ee_amplitude, oh_amplitude, oo_amplitude, th_amplitude);
        // Get viseme level
        calculateAmplitude(ah_amplitude, ee_amplitude, oh_amplitude, oo_amplitude, th_amplitude, min_amplitude, max_amplitude, avg_amplitude);
        levelBoost(dominantViseme, max_amplitude);
        loudness_level = calculateLoudness(max_amplitude, avg_amplitude);
        loudness_level = max_amplitude > noiseThreshold ? loudness_level : 0;
        loudness_level = smoothedLoudness(loudness_level);

        //? Debugging
        // int max_threshold = 2000;
        // Serial.print("Max_THRESHOLD:");
        // Serial.print(max_threshold);
        // Serial.print(",noiseThreshold:");
        // Serial.print(noiseThreshold);
        // Serial.print(",AH:");
        // Serial.print(ah_amplitude > max_threshold ? max_threshold : ah_amplitude);
        // Serial.print(",EE:");
        // Serial.print(ee_amplitude > max_threshold ? max_threshold : ee_amplitude);
        // Serial.print(",OH:");
        // Serial.print(oh_amplitude > max_threshold ? max_threshold : oh_amplitude);
        // Serial.print(",OO:");
        // Serial.print(oo_amplitude > max_threshold ? max_threshold : oo_amplitude);
        // Serial.print(",TH:");
        // Serial.println(th_amplitude > max_threshold ? max_threshold : th_amplitude);
        // Serial.print(",Level:");
        // Serial.print(loudness_level * 100);
        // Serial.print(",Max:");
        // Serial.print(max_amplitude > max_threshold ? max_threshold : max_amplitude);
        // Serial.print(",AVG_AMP:");
        // Serial.println(avg_amplitude > max_threshold ? max_threshold : avg_amplitude);

        //! Final render
        return visemeOutput(dominantViseme, loudness_level);
    }

    double getNoiseThreshold() {
        return noiseThreshold;
    };

    void setNoiseThreshold(double value) {
        noiseThreshold = value;
    };

   private:
    I2SMicrophone microphone = I2SMicrophone();
    arduinoFFT FFT = arduinoFFT(real, imaginary, SAMPLES, SAMPLE_RATE);
    double noiseThreshold = NOISE_THRESHOLD;

    enum VisemeType {
        AH,
        EE,
        OH,
        OO,
        TH
    };
    const uint8_t* ahViseme[20] = {
        mouthAH1, mouthAH2, mouthAH3, mouthAH4, mouthAH5, mouthAH6, mouthAH7, mouthAH8, mouthAH9, mouthAH10,
        mouthAH11, mouthAH12, mouthAH13, mouthAH14, mouthAH15, mouthAH16, mouthAH17, mouthAH18, mouthAH19, mouthAH20};
    const uint8_t* eeViseme[20] = {
        mouthEE1, mouthEE2, mouthEE3, mouthEE4, mouthEE5, mouthEE6, mouthEE7, mouthEE8, mouthEE9, mouthEE10,
        mouthEE11, mouthEE12, mouthEE13, mouthEE14, mouthEE15, mouthEE16, mouthEE17, mouthEE18, mouthEE19, mouthEE20};
    const uint8_t* ohViseme[20] = {
        mouthOH1, mouthOH2, mouthOH3, mouthOH4, mouthOH5, mouthOH6, mouthOH7, mouthOH8, mouthOH9, mouthOH10,
        mouthOH11, mouthOH12, mouthOH13, mouthOH14, mouthOH15, mouthOH16, mouthOH17, mouthOH18, mouthOH19, mouthOH20};
    const uint8_t* ooViseme[20] = {
        mouthOO1, mouthOO2, mouthOO3, mouthOO4, mouthOO5, mouthOO6, mouthOO7, mouthOO8, mouthOO9, mouthOO10,
        mouthOO11, mouthOO12, mouthOO13, mouthOO14, mouthOO15, mouthOO16, mouthOO17, mouthOO18, mouthOO19, mouthOO20};
    const uint8_t* thViseme[20] = {
        mouthTH1, mouthTH2, mouthTH3, mouthTH4, mouthTH5, mouthTH6, mouthTH7, mouthTH8, mouthTH9, mouthTH10,
        mouthTH11, mouthTH12, mouthTH13, mouthTH14, mouthTH15, mouthTH16, mouthTH17, mouthTH18, mouthTH19, mouthTH20};

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

    void normalizeViseme(double& ah_amplitude, double& ee_amplitude, double& oh_amplitude, double& oo_amplitude, double& th_amplitude) {
        ah_amplitude *= 0.6;
        ee_amplitude *= 1.0;
        oh_amplitude *= 1.8;
        oo_amplitude *= 2.2;
        th_amplitude *= 2.6;
    }

    VisemeType getDominantViseme(double ah_amplitude, double ee_amplitude, double oh_amplitude, double oo_amplitude, double th_amplitude) {
        VisemeType viseme = AH, second_viseme = AH;
        double viseme_amplitude = ah_amplitude, second_amplitude = ah_amplitude;

        if (ee_amplitude > viseme_amplitude) {
            if (ee_amplitude >= second_amplitude) {
                second_viseme = viseme;
                second_amplitude = viseme_amplitude;

                viseme = EE;
                viseme_amplitude = ee_amplitude;
            }
        }
        if (oh_amplitude > viseme_amplitude) {
            if (oh_amplitude >= second_amplitude) {
                second_viseme = viseme;
                second_amplitude = viseme_amplitude;

                viseme = OH;
                viseme_amplitude = oh_amplitude;
            }
        }
        if (oo_amplitude > viseme_amplitude) {
            if (oo_amplitude >= second_amplitude) {
                second_viseme = viseme;
                second_amplitude = viseme_amplitude;

                viseme = OO;
                viseme_amplitude = oo_amplitude;
            }
        }
        if (th_amplitude > viseme_amplitude) {
            if (th_amplitude >= second_amplitude) {
                second_viseme = viseme;
                second_amplitude = viseme_amplitude;

                viseme = TH;
            }
        }

        // If 'AH' is still the highest viseme and the second viseme is not too far from it, return the second viseme
        if (viseme == AH && second_amplitude >= ah_amplitude * 0.75) {
            viseme = second_viseme;
        }
        return viseme;
    }

    void calculateAmplitude(double ah, double ee, double oh, double oo, double th, double& minAmp, double& maxAmp, double& avgAmp) {
        minAmp = min(min(min(min(ah, ee), oh), oo), th);
        maxAmp = max(max(max(max(ah, ee), oh), oo), th);
        avgAmp = (ah + ee + oh + oo + th) / 5.0;
    }
    // Compute loudness level based on average amplitude
    unsigned int calculateLoudness(double max, double avg) {
        return mapFloat(max / avg, 1, 2.8, 1, 20);
    }

    void levelBoost(VisemeType viseme, double& maxAmp) {
        if (viseme != AH) {
            maxAmp *= 1.5;
        }
    }

    VisemeType previousViseme;
    VisemeType holdViseme(VisemeType input) {
        if (input != 0) {
            return input;
        }
        previousViseme = input;
        return previousViseme;
    }
    unsigned int currentLoudness = 0;
    unsigned long decayStartTime = 0;
    const unsigned long decayElapsedThreshold = 1000;
    const double decayRate = 0.0003;  // Adjusted decay rate (units per millisecond)

    unsigned int smoothedLoudness(unsigned int input) {
        unsigned long currentTime = millis();
        unsigned long decayElapsedTime = currentTime - decayStartTime;

        if (input >= currentLoudness) {
            // Increment limit
            if (input - currentLoudness > 5) {
                input = currentLoudness + 5;
                input = input > 20 ? 20 : input;
            }
            currentLoudness = input;
            decayStartTime = 0;
        } else {
            if (decayStartTime == 0) {
                decayStartTime = currentTime;
            } else {
                if (decayElapsedTime >= decayElapsedThreshold && currentLoudness <= 5) {
                    currentLoudness = 0;
                    decayStartTime = 0;
                } else {
                    unsigned int decayedInput = static_cast<unsigned int>(currentLoudness - (decayRate * decayElapsedTime));
                    if (decayedInput < input) {
                        currentLoudness = input;
                        decayStartTime = 0;
                    } else {
                        currentLoudness = decayedInput;
                    }
                }
            }
        }
        return currentLoudness;
    }

    const uint8_t* visemeOutput(VisemeType viseme, unsigned int level) {
        static int previousLevel = -1;  // Initialize with an invalid value
        if (level == 0) {
            previousLevel = level;
            return mouthDefault;
        }
        if (level < previousLevel) {  // Hold viseme when level decreasing
            viseme = previousViseme;
        } else {
            previousViseme = viseme;
        }
        previousLevel = level;
        level -= 1;
        // Serial.print("Base:");
        // Serial.print(4);
        // Serial.print(",Level:");
        // Serial.print(level);
        // Serial.print(",viseme:");
        // Serial.print(viseme);
        // Serial.print(",previousViseme:");
        // Serial.println(previousViseme);
        // auto combination = visemeCombination.find(std::make_pair(viseme, previousViseme));
        // Serial.print(",combination:");
        // Serial.println(combination->second);
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

    // std::map<std::pair<int, int>, String> visemeCombination = {
    //     {{AH, AH}, "AHViseme"},
    //     {{EE, EE}, "EEViseme"},
    //     {{OH, OH}, "OHViseme"},
    //     {{OO, OO}, "OOViseme"},
    //     {{TH, TH}, "THViseme"},
    //     {{AH, EE}, "AHEEViseme"},
    //     {{AH, OH}, "AHOHViseme"},
    //     {{AH, OO}, "AHOOViseme"},
    //     {{AH, TH}, "AHTHViseme"},
    //     {{EE, AH}, "AHEEViseme"},
    //     {{EE, OH}, "EEOHViseme"},
    //     {{EE, OO}, "EEOOViseme"},
    //     {{EE, TH}, "EETHViseme"},
    //     {{OH, AH}, "AHOHViseme"},
    //     {{OH, EE}, "EEOHViseme"},
    //     {{OH, OO}, "OHOOViseme"},
    //     {{OH, TH}, "OHTHViseme"},
    //     {{OO, AH}, "AHOOViseme"},
    //     {{OO, EE}, "EEOOViseme"},
    //     {{OO, OH}, "OHOOViseme"},
    //     {{OO, TH}, "OOTHViseme"},
    //     {{TH, AH}, "AHTHViseme"},
    //     {{TH, EE}, "EETHViseme"},
    //     {{TH, OH}, "OHTHViseme"},
    //     {{TH, OO}, "OOTHViseme"}};
};
