#pragma once

#include <APDS9930.h>
#include "config.h"

class APDS9930Sensor {
   private:
    APDS9930 apds = APDS9930();
    bool sensorInitialized = false;
    int proximity_max = 1;

    bool readAPDSSensor(uint16_t &proximityData, float &ambientLight);
    void filterProx(uint16_t &value);
    void conquerTheSun(uint16_t &proximity, float &ambientLight);

   public:
    bool setup();
    void read(uint16_t *proximityData);
};
