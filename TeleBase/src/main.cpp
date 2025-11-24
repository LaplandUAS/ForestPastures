#include "main.h"

response_message responseData;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    Serial.printf("Data received from: %02X:%02X:%02X:%02X:%02X:%02X\n",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    if (!esp_now_is_peer_exist(mac)) {
        Serial.println("New peer detected, adding to peer list...");
        
        esp_now_peer_info_t peerInfo;
        memset(&peerInfo, 0, sizeof(peerInfo));
        memcpy(peerInfo.peer_addr, mac, 6);
        peerInfo.channel = 0;
        peerInfo.encrypt = false;
        
        esp_err_t addResult = esp_now_add_peer(&peerInfo);
        if (addResult == ESP_OK) {
            Serial.println("Peer added successfully");
        } else {
            Serial.printf("Failed to add peer: %d\n", addResult);
        }
    }

    // Assuming the same data structure as the sender
    enum pkType{
    HEAD, DATA, NONE
    };
    typedef struct struct_message {
    pkType type;
    char data[32];
    int32_t inf;
    } struct_message;
    
    struct_message receivedData;
    memcpy(&receivedData, incomingData, sizeof(receivedData));
    
    Serial.printf("Received data:\n");
    Serial.printf("Data:%s\n",receivedData.data);

    String filename = macToFilename(mac);

    // --- append data to file ---
    String line = String(receivedData.data) + "\n";
    m_mStorage.AppendFile(filename.c_str(), line.c_str());

    if (receivedData.inf >= 0) {
        Serial.printf("inf: Line %d\n", receivedData.inf);
    } else {
        Serial.println("inf: TXEND");
        //Reply with timestamp
        time_t now = time(nullptr);
        responseData.unixTimestamp = (uint32_t)now;
        responseData.status = 0; // Success
        strcpy(responseData.message, "TIME_SYNC");
        
        Serial.printf("Sending time sync: %lu (Unix timestamp)\n", responseData.unixTimestamp);
        
        // Send response back to sender
        esp_err_t result = esp_now_send(mac, (uint8_t*)&responseData, sizeof(responseData));
        
        if (result == ESP_OK) {
            Serial.println("Time sync response sent successfully");
        } else {
            Serial.printf("Error sending response: %d\n", result);
        }
    }
    Serial.println("Data processing complete\n");
    Serial.println();
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.printf("Time sync response send status: ");
    Serial.printf(status == ESP_NOW_SEND_SUCCESS ? "Success\n" : "Failed\n");
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Base Station BLE Advertiser & ESP-NOW Receiver...");
    UpdateTime();
    if(!m_mStorage.InitCard())
    {
        return;
    }
        // Initialize WiFi to get MAC address
    WiFi.mode(WIFI_STA);
    String baseMAC = WiFi.macAddress();
    Serial.printf("Base Station WiFi MAC: %s\n", baseMAC.c_str());
    Serial.printf("Base Station BLE MAC will be: %s (WiFi MAC + 2)\n", baseMAC.c_str());
    Serial.println("Battery devices should use WiFi MAC for ESP-NOW and BLE MAC for proximity detection");
    
    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    
    // Attach callbacks & stuff
    esp_now_register_recv_cb(OnDataRecv);
    esp_now_register_send_cb(OnDataSent);
    Serial.println("ESP-NOW receiver initialized");
    
    // Initialize BLE
    BLEDevice::init("BaseStation");
    
    // Create BLE Server
    BLEServer* pServer = BLEDevice::createServer();
    
    // Create BLE Service
    BLEService* pService = pServer->createService("12345678-1234-1234-1234-123456789abc");
    pService->start();
    
    // Get advertising obj
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    
    // Add service UUID to advertising data
    pAdvertising->addServiceUUID("12345678-1234-1234-1234-123456789abc");
    
    // Set advertising params
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);    //This is for iPhone stuff
    pAdvertising->setMaxPreferred(0x12);
    
    // Start advertising
    BLEDevice::startAdvertising();
    
    Serial.println("BLE advertising started!");
    Serial.println("Device name: BaseStation");
    Serial.println("Ready to receive ESP-NOW data and provide BLE proximity detection...");
}

void loop() {
    // Just keep advertising and receiving - nothing else needed
    delay(1000);
    
    // Acknowledge existence
    static unsigned long lastStatus = 0;
    if (millis() - lastStatus > 30000) {
        Serial.println("Still advertising BLE and listening for ESP-NOW data...");
        lastStatus = millis();
    }
}