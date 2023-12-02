#include <esp_now.h>
#include <WiFi.h>

//? ------------------------ ESP NOW ------------------------
uint8_t masterMacAddress[] = {0x84, 0xFC, 0xE6, 0x00, 0x26, 0x7C};
static const char* PMK_KEY_STR = "NHkeBaL5YkoAUsi6";
static const char* LMK_KEY_STR = "eYF8CUjnkFq3Ke5f";

typedef struct struct_message {
    uint16_t debugLed;
    int b;
} struct_message;
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
    memcpy(&myData, incomingData, sizeof(myData));
    // Serial.print("Bytes received: ");
    // Serial.print(len);
    // Serial.print(", Int: ");
    // Serial.println(myData.b);
    // Serial.println();
    controller.updatePixelPosition(myData.debugLed);
    controller.recieveEspNow(myData.b);
}

void setupEspNow() {
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    // Set the PMK key
    esp_now_set_pmk((uint8_t*)PMK_KEY_STR);

    // Register the master as peer
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, masterMacAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = true;
    // Setting the master device LMK key
    for (uint8_t i = 0; i < 16; i++) {
        peerInfo.lmk[i] = LMK_KEY_STR[i];
    }

    // Add master as peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);
}