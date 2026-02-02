#include "Viseme.h"
#include <vector>

void Viseme::initMic() {
    mic.init(i2sSampleRate, i2sSamples);
}

float Viseme::getNoiseThreshold() {
    return noiseThreshold;
}

void Viseme::setNoiseThreshold(float value) {
    noiseThreshold = value;
}

void Viseme::getDigtalSample(double* vReal, double* vImagine, bool isSmooth) {
    int16_t buffer[i2sSamples];
    mic.read(buffer, i2sSamples);
    float smoothedValue = 0;
    for (int i = 0; i < i2sSamples; i++) {
        if (isSmooth) {
            smoothedValue = alpha * buffer[i] + (1 - alpha) * smoothedValue;  // apply exponential smoothing
            vReal[i] = smoothedValue;
        } else {
            vReal[i] = buffer[i];
        }
        vImagine[i] = 0;
    }
}

void Viseme::getAnalogSample(double* vReal, double* vImagine, bool isSmooth) {
    // unsigned int sampling_period_us = round(1000 * (1.0 / i2sSampleRate));
    // unsigned long microseconds;
    // double smoothedSample = 0;

    // for (int i = 0; i < i2sSamples; i++) {
    //     microseconds = micros();

    //     double sample = analogRead(microphone_pin);
    //     if (isSmooth) {
    //         smoothedSample = alpha * sample + (1 - alpha) * smoothedSample;
    //         vReal[i] = smoothedSample;
    //     } else {
    //         vReal[i] = sample;
    //     }
    //     vImagine[i] = 0;

    //     while (micros() < (microseconds + sampling_period_us)) {
    //     }
    // }
}

void Viseme::calcFFT() {
    FFT.dcRemoval();
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
    FFT.compute(FFTDirection::Forward);
    FFT.complexToMagnitude();
}

void Viseme::calculateAmplitude(double ah, double ee, double oh, double oo, double th, double& minAmp, double& maxAmp, double& avgAmp) {
    minAmp = min(min(ah, ee), min(min(oh, oo), th));
    maxAmp = max(max(ah, ee), max(max(oh, oo), th));
    avgAmp = (ah + ee + oh + oo + th) / 5.0;
}

void Viseme::normalizeViseme(double& ah_amplitude, double& ee_amplitude, double& oh_amplitude, double& oo_amplitude, double& th_amplitude) {
    ah_amplitude *= 0.6;
    ee_amplitude *= 1.0;
    oh_amplitude *= 1.8;
    oo_amplitude *= 2.2;
    th_amplitude *= 2.6;
}

void Viseme::levelBoost(Viseme::VisemeType viseme, double& maxAmp) {
    if (viseme != AH) {
        maxAmp *= 1.5;
    }
}

unsigned int Viseme::calculateLoudness(double max, double avg) {
    return mapFloat(max / avg, 0.6, 2.8, 1, visemeFramelength);
}

unsigned int Viseme::smoothedLoudness(unsigned int input) {
    unsigned long currentTime = millis();
    unsigned long decayElapsedTime = currentTime - decayStartTime;

    if (input >= currentLoudness) {
        // Increment limit
        if (input - currentLoudness > 5) {
            input = currentLoudness + 5;
            input = input > visemeFramelength ? visemeFramelength : input;
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

Viseme::VisemeType Viseme::getDominantViseme(double ah_amplitude, double ee_amplitude, double oh_amplitude, double oo_amplitude, double th_amplitude) {
    std::map<double, VisemeType> amplitudeToViseme = {
        {ah_amplitude, AH},
        {ee_amplitude, EE},
        {oh_amplitude, OH},
        {oo_amplitude, OO},
        {th_amplitude, TH},
    };

    // Sort the map in descending order of amplitude
    std::vector<std::pair<double, VisemeType>> sortedAmplitudes(amplitudeToViseme.begin(), amplitudeToViseme.end());
    std::sort(sortedAmplitudes.begin(), sortedAmplitudes.end(), std::greater<std::pair<double, VisemeType>>());

    // Get the viseme with the highest amplitude
    VisemeType viseme = sortedAmplitudes[0].second;

    // If 'AH' is still the highest viseme and the second viseme is not too far from it, return the second viseme
    if (viseme == AH && sortedAmplitudes[1].first >= sortedAmplitudes[0].first * 0.75) {
        viseme = sortedAmplitudes[1].second;
    }

    return viseme;
}

Viseme::VisemeType Viseme::holdViseme(VisemeType input) {
    VisemeType held_viseme = previousViseme;
    if (input != 0) {
        held_viseme = input;
    }
    previousViseme = held_viseme;
    return held_viseme;
}

const uint8_t* Viseme::visemeOutput(VisemeType viseme, unsigned int level) {
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
    return nullptr;
}

const uint8_t* Viseme::renderViseme() {
    getDigtalSample(real, imaginary, true);
    // getAnalogSample(real, imaginary, false);
    calcFFT();

    // Compute amplitude of frequency ranges for each viseme
    double ah_amplitude = 0, ee_amplitude = 0,
           oh_amplitude = 0, oo_amplitude = 0, th_amplitude = 0;
    double min_amplitude, max_amplitude, avg_amplitude;
    unsigned int loudness_level;

    for (int i = 4; i < i2sSamples / 2; i++) {
        double freq = i * ((i2sSampleRate / 2.0) / (i2sSamples / 2.0));
        double amplitude = abs(imaginary[i]);
        if (freq >= visemeAhFreqMin && freq <= visemeAhFreqMax) {
            ah_amplitude += amplitude;
        }
        if (freq >= visemeEeFreqMin && freq <= visemeEeFreqMax) {
            ee_amplitude += amplitude;
        }
        if (freq >= visemeOhFreqMin && freq <= visemeOhFreqMax) {
            oh_amplitude += amplitude;
        }
        if (freq >= visemeOoFreqMin && freq <= visemeOoFreqMax) {
            oo_amplitude += amplitude;
        }
        if (freq >= visemeThFreqMin && freq <= visemeThFreqMax) {
            th_amplitude += amplitude;
        }
    }
    // Postprocessing
    normalizeViseme(ah_amplitude, ee_amplitude, oh_amplitude, oo_amplitude, th_amplitude);
    // Find dominant viseme
    VisemeType dominantViseme = getDominantViseme(ah_amplitude, ee_amplitude, oh_amplitude, oo_amplitude, th_amplitude);
    levelBoost(dominantViseme, max_amplitude);
    // Get viseme level
    calculateAmplitude(ah_amplitude, ee_amplitude, oh_amplitude, oo_amplitude, th_amplitude, min_amplitude, max_amplitude, avg_amplitude);
    loudness_level = calculateLoudness(max_amplitude, avg_amplitude);
    loudness_level = max_amplitude > noiseThreshold ? loudness_level : 0;
    loudness_level = smoothedLoudness(loudness_level);

    // Debugging
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