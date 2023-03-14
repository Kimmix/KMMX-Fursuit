#include <ArduinoBLE.h>

#define BLE_SERVICE_UUID "c1449275-bf34-40ab-979d-e34a1fdbb129"
#define BLE_CHARACTERISTIC_UUID "49a36bb2-1c66-4e5c-8ff3-28e55a64beb3"

BLEService service(BLE_SERVICE_UUID);
BLECharacteristic brightnessCharacteristic(BLE_CHARACTERISTIC_UUID, BLERead | BLEWrite, 0);

class BluetoothController {
private:
    DisplayController* display;

    int brightnessValue;
    bool isConnected;

public:
    BluetoothController(DisplayController* displayPtr = nullptr): display(displayPtr) {}

    void init() {
        isConnected = false;
        Serial.println("Booting BLE...");
        delay(500);
        if (!BLE.begin()) {
            Serial.println("failed to initialize BLE!");
            while (1);
        }
        Serial.println("initialize BLE...");

        // Define the BLE service and characteristic
        service.addCharacteristic(brightnessCharacteristic);
        BLE.addService(service);
        BLE.setDeviceName("KMMX");
        BLE.setLocalName("KMMX-BLE");

        // Start advertising the BLE service
        BLE.advertise();
        Serial.println("Bluetooth® device active, waiting for connections...");
    }

    void update() {
        BLEDevice central = BLE.central();
        if (central) {
            isConnected = true;
            digitalWrite(LED_BUILTIN, HIGH);
            brightnessValue = display->getBrightnessValue();
            while (central.connected()) {
                if (brightnessCharacteristic.written()) {
                    // Read the new brightness value from the characteristic
                    brightnessValue = brightnessCharacteristic.value()[0];
                    // Do something with the new brightness value, e.g. set the display brightness
                    // setBrightness(brightnessValue);
                    display->setBrightnessValue(brightnessValue);
                }

                // Notify the central device of the current brightness value
                brightnessCharacteristic.writeValue(brightnessValue);
            }

            // Disconnect from the central device
            central.disconnect();
            isConnected = false;
            digitalWrite(LED_BUILTIN, LOW);
        }
    }

    bool getIsConnected() {
        return isConnected;
    }

    int getBrightnessValue() {
        return brightnessValue;
    }
};
