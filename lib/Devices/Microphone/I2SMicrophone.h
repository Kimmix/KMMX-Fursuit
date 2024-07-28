#pragma once

#include <driver/i2s.h>
#include <Arduino.h>
#include "Config.h"

class I2SMicrophone {
   private:
    void i2s_install(uint32_t sampleRate, int samples);
    void i2s_setpin();

   public:
    I2SMicrophone();
    ~I2SMicrophone();
    void init(uint32_t sampleRate, int samples);
    void read(int16_t* buffer, int samples);
    void start_i2s();
    void stop_i2s();
};