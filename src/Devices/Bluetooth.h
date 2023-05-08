#include <ArduinoBLE.h>

#define LED_BUILTIN GPIO_NUM_2
#define BLE_SERVICE_UUID "c1449275-bf34-40ab-979d-e34a1fdbb129"
#define BLE_CHARACTERISTIC_UUID "49a36bb2-1c66-4e5c-8ff3-28e55a64beb3"

BLEService service(BLE_SERVICE_UUID);
BLEByteCharacteristic brightnessCharacteristic(BLE_CHARACTERISTIC_UUID, BLERead | BLEWrite);

class BluetoothController {
   public:
    BluetoothController(LEDMatrixDisplay* displayPtr = nullptr) : display(displayPtr) {}

    void init() {
        Serial.println("Booting BLE...");
        pinMode(LED_BUILTIN, OUTPUT);
        if (!BLE.begin()) {
            Serial.println("failed to initialize BLE!");
            while (1);
        }
        BLE.setDeviceName("KMMX");
        BLE.setLocalName("KMMX-BLE");

        // Define the BLE service and characteristic
        BLE.setAdvertisedService(service);
        service.addCharacteristic(brightnessCharacteristic);

        BLE.addService(service);
        BLE.setEventHandler(BLEConnected, BluetoothController::blePeripheralConnectHandler);
        BLE.setEventHandler(BLEDisconnected, BluetoothController::blePeripheralDisconnectHandler);

        // Start advertising the BLE service
        BLE.advertise();
        Serial.println("Bluetooth® device active, waiting for connections...");
    }

    void update() {
        BLE.poll();  // Start BLE
        if (brightnessCharacteristic.written()) {
            display->setBrightnessValue(brightnessCharacteristic.value());
        }
    }

   private:
    LEDMatrixDisplay* display;

    int getBrightnessValue() {
        return display->getBrightnessValue();
    }

    // On bluetooth connected
    static void blePeripheralConnectHandler(BLEDevice central) {
        Serial.print("Connected event, central: ");
        Serial.println(central.address());
        digitalWrite(LED_BUILTIN, HIGH);
    }
    // On bluetooth disconnected
    static void blePeripheralDisconnectHandler(BLEDevice central) {
        Serial.print("Disconnected event, central: ");
        Serial.println(central.address());
        digitalWrite(LED_BUILTIN, LOW);
    }
    // void switchCharacteristicWritten(BLEDevice central,
    //     BLECharacteristic characteristic) {
    //     Serial.print("Characteristic event, written: ");
    //     Serial.println(brightnessCharacteristic.value()[0]);
    //     // Serial.print(brightnessCharacteristic.value()[0]);
    //     // display->setBrightnessValue(brightnessCharacteristic.value());
    // }
};
