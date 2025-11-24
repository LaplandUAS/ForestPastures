#ifndef WIRELESSTRANSFER_H
#define WIRELESSTRANSFER_H
#include <Arduino.h>
#include <stdint.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <FS.h>
#include <ESP32Time.h>
#include "debugtypes.h"

enum pkType{
  HEAD, DATA, NONE
};
typedef struct struct_message {
  pkType type;
  char data[32];
  int32_t inf;
} struct_message;

typedef struct response_message {
    uint32_t unixTimestamp;  // Unix time from base station
    uint8_t status;
    char message[32];
} response_message;

//94:B9:7E:66:12:AC is base station
extern uint8_t broadcastAddress[6];
extern struct_message myData;
extern esp_now_peer_info_t peerInfo;

void BeginLink();
void SetRTCReference(ESP32Time* rtcPtr);
void OnDataSent(const uint8_t*mac_addr, esp_now_send_status_t  status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
bool SendData(const struct_message myData);
void CleanupLink();
bool ResponseReceived();
bool IsDataTransferComplete();

#endif