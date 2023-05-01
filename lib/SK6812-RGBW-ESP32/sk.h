// * Source: https://github.com/gitpeut/SK6812-RGBW-ESP32
#ifndef SK_H
#define SK_H 1

#include "driver/rmt.h"

// Use RMT for a strip of SK6812 RGBW leds

class sk {
   public:
    sk(void) {}
    ~sk(void) {
        free(_skstrip);
        rmt_driver_uninstall(skconfig.channel);
    }

    /*-------------------------------------------------------------------------*/
    int begin(int pin, int count) {
        _bitcount = count * 32;
        _ledpin = pin;
        _ledcount = count;
        _brightness = 100;

        int rc;

        _skstrip = (rmt_item32_t *)calloc(_bitcount, sizeof(rmt_item32_t));

        skconfig.rmt_mode = RMT_MODE_TX;
        skconfig.channel = RMT_CHANNEL_7;
        skconfig.gpio_num = (gpio_num_t)_ledpin;
        skconfig.mem_block_num = 1;
        skconfig.tx_config.loop_en = 0;
        skconfig.tx_config.carrier_en = 0;
        skconfig.tx_config.idle_output_en = 1;
        skconfig.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
        skconfig.tx_config.carrier_level = RMT_CARRIER_LEVEL_HIGH;
        skconfig.clk_div = 8;  // 80MHx / 8 = 10MHz translates to  0,1 us = 100 ns per count

        rmt_config(&skconfig);
        rc = rmt_driver_install(skconfig.channel, 0, 0);  //  rmt_driver_install(rmt_channel_t channel, size_t rx_buf_size, int rmt_intr_num)

        return (rc);
    }

    /*-------------------------------------------------------------------------*/
    uint32_t getcolor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
        uint32_t kleur = 0;

        kleur |= ((uint32_t)g << 24);
        kleur |= ((uint32_t)r << 16);
        kleur |= ((uint32_t)b << 8);
        kleur |= (uint32_t)w;

        return (kleur);
    }

    /*-------------------------------------------------------------------------*/

    uint32_t breakcolor(uint32_t kleur, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *w) {
        *g = (kleur >> 24) & 0xff;
        *r = (kleur >> 16) & 0xff;
        *b = (kleur >> 8) & 0xff;
        *w = kleur & 0xff;

        return (kleur);
    }

    /*-------------------------------------------------------------------------*/
    void color32(int led, uint32_t kleur, int brightness = -1) {
        uint8_t r, g, b, w;

        breakcolor(kleur, &r, &g, &b, &w);
        color(led, r, g, b, w, brightness);
    }

    /*-------------------------------------------------------------------------*/
    void color(int led, uint8_t red, uint8_t green, uint8_t blue, uint8_t white, int brightness = -1) {
        const float r_correction = 1.2;  // example color correction coefficients
        const float g_correction = 0.9;
        const float b_correction = 1.1;
        const float w_correction = 1.0;

        // apply color correction
        float r_corrected = red * r_correction;
        float g_corrected = green * g_correction;
        float b_corrected = blue * b_correction;
        float w_corrected = white * w_correction;

        // adjust brightness
        float brightness_float = brightness == -1 ? 1.0 : brightness / 100.0;
        brightness_float *= _brightness / 100.0;

        // scale colors based on brightness
        r_corrected *= brightness_float;
        g_corrected *= brightness_float;
        b_corrected *= brightness_float;
        w_corrected *= brightness_float;

        // convert colors to integers
        uint8_t r = (uint8_t)r_corrected;
        uint8_t g = (uint8_t)g_corrected;
        uint8_t b = (uint8_t)b_corrected;
        uint8_t w = (uint8_t)w_corrected;

        // create 32-bit color value
        uint32_t color = 0;
        color |= ((uint32_t)g << 24);
        color |= ((uint32_t)r << 16);
        color |= ((uint32_t)b << 8);
        color |= (uint32_t)w;

        // set color for the specified LED
        if (led >= 0 && led < ledcount()) {
            for (int i = (led * 32), bit = 0; bit < 32; bit++) {
                if ((color & (1 << (31 - bit)))) {
                    _skstrip[i].level0 = 1;
                    _skstrip[i].duration0 = 6;
                    _skstrip[i].level1 = 0;
                    _skstrip[i].duration1 = 6;
                } else {
                    _skstrip[i].level0 = 1;
                    _skstrip[i].duration0 = 3;
                    _skstrip[i].level1 = 0;
                    _skstrip[i].duration1 = 9;
                }
                ++i;
            }
        }
    }

    /*-------------------------------------------------------------------------*/
    void show() {
        int rc;
        // esp_err_t rmt_write_items(rmt_channel_t channel, rmt_item32_t *rmt_item, int item_num, bool wait_tx_done)

        rc = rmt_write_items(skconfig.channel, _skstrip, _bitcount, 1);

        delay(2);
    }

    /*-------------------------------------------------------------------------*/

    void clear(void) {
        for (int i = 0; i < _ledcount; ++i) {
            color(i, 0, 0, 0, 0);
        }

        show();
        delay(10);
    }

    /*-------------------------------------------------------------------------*/
    uint8_t setbrightness(uint8_t newbrightness) {
        _brightness = newbrightness;
        return (_brightness);
    }
    /*-------------------------------------------------------------------------*/
    uint8_t getbrightness() {
        return (_brightness);
    }
    /*-------------------------------------------------------------------------*/

    int ledcount() {
        return (_ledcount);
    }

    /*-------------------------------------------------------------------------*/

   private:
    int _ledcount;
    int _bitcount;
    int _ledpin;
    int _brightness;
    rmt_item32_t *_skstrip = NULL;
    rmt_config_t skconfig;
    float _realtick;
};

#endif
