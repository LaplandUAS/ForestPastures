#ifndef NETSCAN_H
#define NETSCAN_H
#include <Arduino.h>
#include "BLEDevice.h"
#include "BLEAdvertisedDevice.h"
#include "BLEScan.h"

static bool bleInitialized=false;


class ProximityScanner : public BLEAdvertisedDeviceCallbacks {
private:
    bool baseStationFound = false;
    int rssiThreshold = -75; // About ~5 meters
    std::string targetMAC = "94:b9:7e:66:12:ae";
    
public:
    void onResult(BLEAdvertisedDevice advertisedDevice);
    bool isBaseStationNear() { return baseStationFound; }
    void reset() { baseStationFound = false; }
};

bool checkProximity();
#endif