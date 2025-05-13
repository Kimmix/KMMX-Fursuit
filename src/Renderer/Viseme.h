#pragma once

#include <map>
#include <arduinoFFT.h>
#include "Utils/Utils.h"
#include "config.h"
#include "Devices/Microphone/I2SMicrophone.h"
#include "Bitmaps/Bitmaps.h"

class Viseme {
   public:
    void initMic();
    const uint8_t* renderViseme();
    double getNoiseThreshold();
    void setNoiseThreshold(double value);

   private:
    enum VisemeType {
        AH,
        EE,
        OH,
        OO,
        TH
    };
    I2SMicrophone mic;
    ArduinoFFT<double> FFT = ArduinoFFT<double>(real, imaginary, i2sSamples, i2sSampleRate);
    double real[i2sSamples], imaginary[i2sSamples];
    double noiseThreshold = visemeNoiseThreshold;
    const float alpha = visemeSmoothingAlpha;
    const double decayRate = visemeDecayRate;  // Adjusted decay rate (units per millisecond)
    unsigned int currentLoudness = 0;
    unsigned long decayStartTime = 0;
    const unsigned long decayElapsedThreshold = 1000;
    static const short visemeFramelength = 20;
    VisemeType previousViseme;

    const uint8_t* ahViseme[visemeFramelength] = {
        mouthAH1, mouthAH2, mouthAH3, mouthAH4, mouthAH5, mouthAH6, mouthAH7, mouthAH8, mouthAH9, mouthAH10,
        mouthAH11, mouthAH12, mouthAH13, mouthAH14, mouthAH15, mouthAH16, mouthAH17, mouthAH18, mouthAH19, mouthAH20};
    const uint8_t* eeViseme[visemeFramelength] = {
        mouthEE1, mouthEE2, mouthEE3, mouthEE4, mouthEE5, mouthEE6, mouthEE7, mouthEE8, mouthEE9, mouthEE10,
        mouthEE11, mouthEE12, mouthEE13, mouthEE14, mouthEE15, mouthEE16, mouthEE17, mouthEE18, mouthEE19, mouthEE20};
    const uint8_t* ohViseme[visemeFramelength] = {
        mouthOH1, mouthOH2, mouthOH3, mouthOH4, mouthOH5, mouthOH6, mouthOH7, mouthOH8, mouthOH9, mouthOH10,
        mouthOH11, mouthOH12, mouthOH13, mouthOH14, mouthOH15, mouthOH16, mouthOH17, mouthOH18, mouthOH19, mouthOH20};
    const uint8_t* ooViseme[visemeFramelength] = {
        mouthOO1, mouthOO2, mouthOO3, mouthOO4, mouthOO5, mouthOO6, mouthOO7, mouthOO8, mouthOO9, mouthOO10,
        mouthOO11, mouthOO12, mouthOO13, mouthOO14, mouthOO15, mouthOO16, mouthOO17, mouthOO18, mouthOO19, mouthOO20};
    const uint8_t* thViseme[visemeFramelength] = {
        mouthTH1, mouthTH2, mouthTH3, mouthTH4, mouthTH5, mouthTH6, mouthTH7, mouthTH8, mouthTH9, mouthTH10,
        mouthTH11, mouthTH12, mouthTH13, mouthTH14, mouthTH15, mouthTH16, mouthTH17, mouthTH18, mouthTH19, mouthTH20};

    void getAnalogSample(double* vReal, double* vImagine, bool isSmooth);
    void getDigtalSample(double* vReal, double* vImagine, bool isSmooth);
    void calcFFT();
    void calculateAmplitude(double ah, double ee, double oh, double oo, double th, double& minAmp, double& maxAmp, double& avgAmp);
    void normalizeViseme(double& ah_amplitude, double& ee_amplitude, double& oh_amplitude, double& oo_amplitude, double& th_amplitude);
    void levelBoost(VisemeType viseme, double& maxAmp);
    unsigned int calculateLoudness(double max, double avg);
    unsigned int smoothedLoudness(unsigned int input);
    VisemeType getDominantViseme(double ah_amplitude, double ee_amplitude, double oh_amplitude, double oo_amplitude, double th_amplitude);
    VisemeType holdViseme(VisemeType input);
    const uint8_t* visemeOutput(VisemeType viseme, unsigned int level);
};
