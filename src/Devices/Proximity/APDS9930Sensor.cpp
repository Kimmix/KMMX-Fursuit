#include "APDS9930Sensor.h"

bool APDS9930Sensor::readAPDSSensor(uint16_t &proximityData, float &ambientLight) {
    if (apds.readProximity(proximityData) && apds.readAmbientLightLux(ambientLight)) {
        return true;
    }
    return false;
}

void APDS9930Sensor::filterProx(uint16_t &value) {
    const int min_value = 150;
    const int max_value = 50000;
    value -= 200;
    if (value > max_value) {
        value = 0;
    }
    if (value > proximity_max) {
        proximity_max = value;
    }
    if (value < min_value) {
        value = 0;
    }
    value = map(value, 0, proximity_max, 0, 1023);
}

void APDS9930Sensor::conquerTheSun(uint16_t &proximity, float &ambientLight) {
    const float sunlight_threshold = 1000.0;
    if (ambientLight > sunlight_threshold) {
        proximity = 0;
    }
}

bool APDS9930Sensor::setup() {
    if (apds.init()) {
        Serial.println(F("APDS-9930 initialization complete"));
        sensorInitialized = true;
    } else {
        Serial.println(F("Something went wrong during APDS-9930 init!"));
        sensorInitialized = false;
    }

    if (sensorInitialized) {
        if (!apds.setProximityGain(PGAIN_2X)) {
            Serial.println(F("Something went wrong trying to set PGAIN"));
        }

        if (apds.enableProximitySensor(false)) {
            Serial.println(F("Proximity sensor is now running"));
        } else {
            Serial.println(F("Something went wrong during sensor init!"));
        }

        if (apds.enableLightSensor(false)) {
            Serial.println(F("Light sensor is now running"));
        } else {
            Serial.println(F("Something went wrong during light sensor init!"));
        }
    }
    return sensorInitialized;
}

void APDS9930Sensor::read(uint16_t *proximityData) {
    if (!sensorInitialized) {
        return;
    }
    float ambientLight;

    if (readAPDSSensor(*proximityData, ambientLight)) {
        conquerTheSun(*proximityData, ambientLight);
        // Optionally apply filtering
        // filterProx(*proximityData);
        // Serial.print(F("AmbientLight: "));
        // Serial.print(ambientLight);
        // Serial.print(F("Proximity:"));
        // Serial.println(*proximityData);
    } else {
        Serial.println("Error reading sensor data.");
    }
}
