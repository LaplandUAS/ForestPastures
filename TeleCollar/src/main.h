#ifndef MAIN_H
#define MAIN_H

#define BUF_SIZE 32
#define MAX_READ 64
#define MODE_TIMEOUT 250
#define SD_PIN 17
#define ACC_PIN 13  // Not used for interrupts anymore

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <ESP32Time.h>
#include <Wire.h>
#include <SparkFunADXL313.h>
#include <esp_sleep.h>
#include "debugtypes.h"
#include "sensor.h"
#include "netScanner.h"
#include "wirelessTransfer.h"

enum CmdCode {
    EXIT, FLUSH, DELETE, INVALID, PROX, TIME
};

enum SystemState {
    STATE_ACTIVE,
    STATE_SETTLING,
    STATE_SLEEPING
};

// Global variables
ESP32Time rtc{};
Sensor acc;
File logFile;

bool cmdMode = false;
SystemState currentState = STATE_ACTIVE;
int bufIndex = 0;
unsigned long lastMotionTime = 0;
unsigned long tStart;
char rwBuf[BUF_SIZE];
char saveBuffer[MAX_READ][BUF_SIZE];
const uint32_t SLEEP_DURATION = SLEEP_CHECK_INTERVAL;

// Sleep state vars - use RTC data attributes to survive deep sleep
const uint32_t minTransferWait = 600;
RTC_DATA_ATTR uint32_t storedEpoch = 0;
RTC_DATA_ATTR uint32_t storedMillis = 0;
RTC_DATA_ATTR uint32_t lastTransferTime = 0;
RTC_DATA_ATTR bool epochSynced = false;
RTC_DATA_ATTR v3Di sleepBaseline = {0, 0, 0};
RTC_DATA_ATTR bool hasValidBaseline = false;
v3Di lastReading = {0, 0, 0};
int noMotionCount = 0;

// Function declarations
CmdCode hashCommand(String& inString);
void onReadingCycleComplete();
void writeFullBuffer();
void FlushCard();
void ClearLog();
bool checkForMotion(v3Di current, v3Di baseline);
void enterDeepSleep();
void handleWakeup();
void logData(v3Di movement);
void CheckProx();
bool TransferLogFile();


bool parseMAC(const char* macStr, uint8_t* macArray) {
    int values[6];
    
    // Parse hex values separated by colons
    if (sscanf(macStr, "%x:%x:%x:%x:%x:%x", 
               &values[0], &values[1], &values[2], 
               &values[3], &values[4], &values[5]) == 6) {
        
        // Convert to uint8_t array
        for (int i = 0; i < 6; i++) {
            macArray[i] = (uint8_t)values[i];
        }
        return true;
    }
    return false;
}

#endif