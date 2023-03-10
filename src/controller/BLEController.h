#include <ArduinoBLE.h>

// BLE
#define BLE_SERVICE_UUID "c1449275-bf34-40ab-979d-e34a1fdbb129"
#define BLE_CHARACTERISTIC_UUID "49a36bb2-1c66-4e5c-8ff3-28e55a64beb3"

// BLE Service
BLEService kmmxBLEControl(BLE_SERVICE_UUID);

// BLE Characteristic
BLEByteCharacteristic connetionCharacteristic(BLE_CHARACTERISTIC_UUID, BLERead | BLEWrite);

class BLEController {
private:
	// On bluetooth connected
	static void blePeripheralConnectHandler(BLEDevice central) {
		Serial.print("Connected event, central: ");
		Serial.println(central.address());
	}
	// On bluetooth disconnected
	static void blePeripheralDisconnectHandler(BLEDevice central) {
		Serial.print("Disconnected event, central: ");
		Serial.println(central.address());
	}
	static void switchCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
		// central wrote new value to characteristic, update LED
		Serial.print("Characteristic event, written: ");

		if (connetionCharacteristic.value()) {
			Serial.println("LED on");
			digitalWrite(LED_BUILTIN, HIGH);
		}
		else {
			Serial.println("LED off");
			digitalWrite(LED_BUILTIN, LOW);
		}
	}

public:
	void init() {
		// ------ Setup Bluetooth Low Energy ------
		pinMode(LED_BUILTIN, OUTPUT);
		if (!BLE.begin())
			Serial.println("starting Bluetooth® Low Energy module failed!");
		BLE.setDeviceName("KMMX");
		BLE.setLocalName("KMMX-BLE");
		BLE.setAdvertisedService(kmmxBLEControl);
		BLE.addService(kmmxBLEControl);

		// Connection Status LED
		kmmxBLEControl.addCharacteristic(connetionCharacteristic);
		BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
		BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
		connetionCharacteristic.setEventHandler(BLEWritten, switchCharacteristicWritten);
		connetionCharacteristic.setValue(0);

		// Begin 
		BLE.advertise();
		Serial.println(("Bluetooth® device active, waiting for connections..."));
	}
	void start() {
		BLE.poll();
	}
};