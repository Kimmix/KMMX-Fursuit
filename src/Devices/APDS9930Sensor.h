#include <APDS9930.h>

class APDS9930Sensor {
   private:
    APDS9930 apds = APDS9930();
    bool sensorInitialized = false;

    bool readAPDSSensor(uint16_t &proximityData, float &ambientLight) {
        if (apds.readProximity(proximityData) && apds.readAmbientLightLux(ambientLight)) {
            return true;
        }
        return false;
    }

    int proximity_max = 1;
    void filterProx(uint16_t &value) {
        // Adjust the value based on your requirements
        const int min_value = 150,
                  max_value = 50000;
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

    // Disable proximity data if ambient light is too high (e.g., sunlight)
    void conquerTheSun(uint16_t &proximity, float &ambientLight) {
        const float sunlight_threshold = 1000.0;
        if (ambientLight > sunlight_threshold) {
            proximity = 0;
        }
    }

   public:
    void setup() {
        if (apds.init()) {
            Serial.println(F("APDS-9930 initialization complete"));
            sensorInitialized = true;
        } else {
            Serial.println(F("Something went wrong during APDS-9930 init!"));
            sensorInitialized = false;
        }

        if (sensorInitialized) {
            // Adjust the Proximity sensor gain
            if ( !apds.setProximityGain(PGAIN_2X) ) {
            Serial.println(F("Something went wrong trying to set PGAIN"));
            }

            // Start running the APDS-9930 proximity sensor (no interrupts)
            if (apds.enableProximitySensor(false)) {
                Serial.println(F("Proximity sensor is now running"));
            } else {
                Serial.println(F("Something went wrong during sensor init!"));
            }

            // Start running the APDS-9930 light sensor (no interrupts)
            if (apds.enableLightSensor(false)) {
                Serial.println(F("Light sensor is now running"));
            } else {
                Serial.println(F("Something went wrong during light sensor init!"));
            }
        }

    }

    void read(uint16_t *proximityData) {
        if (!sensorInitialized) {
            return;
        }
        // uint16_t proximityData;
        float ambientLight;

        if (readAPDSSensor(*proximityData, ambientLight)) {
            conquerTheSun(*proximityData, ambientLight);
            // filterProx(*proximityData);
            // Serial.print(F("AmbientLight: "));
            // Serial.print(ambientLight);
            // Serial.print(F("Proximity:"));
            // Serial.println(*proximityData);
        } else {
            Serial.println("Error reading sensor data.");
        }
    }
};
