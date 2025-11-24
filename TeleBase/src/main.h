#ifndef MAIN_H
#define MAIN_H

#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"
#include "WiFi.h"
#include "esp_now.h"
#include "nettime.h"
#include "storage.h"

typedef struct response_message {
    uint32_t unixTimestamp;  // Current Unix time from base station
    uint8_t status;          // 0=success, 1=error (UNUSED!)
    char message[32];
} response_message;

MassStorage m_mStorage;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
String macToFilename(const uint8_t *mac) {
    char filename[32];
    snprintf(filename, sizeof(filename), "/%02X%02X%02X%02X%02X%02X.txt",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(filename);
}

#endif