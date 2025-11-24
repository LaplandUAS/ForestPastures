#include "wirelessTransfer.h"
#include <ESP32Time.h>

uint8_t broadcastAddress[] = {0x94, 0xB9, 0x7E, 0x66, 0x12, 0xAC};
struct_message myData;
esp_now_peer_info_t peerInfo;
response_message receivedResponse;

bool espNowInitialized = false;
bool dataTransferComplete = false;
bool responseReceived = false;

ESP32Time* rtcPtr = nullptr;

void BeginLink()
{
    if (espNowInitialized) {
        Serial.println("ESP-NOW already initialized");
        return;
    }
    
    // Make sure WiFi is in station mode
    WiFi.mode(WIFI_STA);
    delay(100); // Give WiFi time to initialize
    
    Serial.printf("Local MAC Address: %s\n", WiFi.macAddress().c_str());
    
    if(esp_now_init() != ESP_OK)
    {
        Serial.printf("Error initializing ESP-NOW\n");
        return;
    }
    
    // Register send & recieve callbacks
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
    
    // Clear peer info structure
    memset(&peerInfo, 0, sizeof(peerInfo));
    
    // Set peer info
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    
    if(esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.printf("Failed to add peer.\n");
        return;
    }
    
    espNowInitialized = true;
    dataTransferComplete = false;
    responseReceived = false;
    Serial.println("ESP-NOW initialized successfully");
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    Serial.printf("Last Packet Send Status: ");
    Serial.printf(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success\n" : "Delivery Fail\n");
    
    if (status == ESP_NOW_SEND_SUCCESS) {
        dataTransferComplete = true;
    }
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    Serial.printf("Response received from base station\n");
    
    if (len == sizeof(response_message)) {
        memcpy(&receivedResponse, incomingData, sizeof(receivedResponse));
        
        Serial.printf("Response details:\n");
        Serial.printf("  Timestamp: %lu\n", receivedResponse.unixTimestamp);
        Serial.printf("  Status: %s\n", receivedResponse.status == 0 ? "Success" : "Error");
        Serial.printf("  Message: %s\n", receivedResponse.message);
        if(rtcPtr != nullptr) rtcPtr->setTime(receivedResponse.unixTimestamp);
        responseReceived = true;
    } else {
        Serial.printf("Received unexpected data size: %d bytes\n", len);
    }
}

bool ResponseReceived()
{
    return responseReceived;
}

void SetRTCReference(ESP32Time* rtc) {
    rtcPtr = rtc;
}

bool SendData(const struct_message myData)
{
    if (!espNowInitialized) {
        Serial.println("ESP-NOW not initialized, call BeginLink() first");
        return false;
    }
    
    // Reset completion flag
    dataTransferComplete = false;
    
    Serial.println("Sending data...");
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));
    
    if (result == ESP_OK) {
        Serial.println("Data sent successfully");
        return true;
    } else {
        Serial.printf("Error sending data: %d\n", result);
        return false;
    }
}

bool IsDataTransferComplete()
{
    return dataTransferComplete;
}

void CleanupLink()
{
    if (espNowInitialized) {
        esp_now_del_peer(broadcastAddress);
        esp_now_deinit();
        espNowInitialized = false;
        Serial.println("ESP-NOW cleaned up");
    }
}