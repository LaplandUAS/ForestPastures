#include "netScanner.h"
void ProximityScanner::onResult(BLEAdvertisedDevice advertisedDevice) {
    // Check if this is our base station
    if (advertisedDevice.getAddress().toString() == targetMAC) {
        int rssi = advertisedDevice.getRSSI();
        Serial.printf("Found base station, RSSI: %d\n", rssi);
        
        if (rssi > rssiThreshold) { // Closer = higher RSSI
            baseStationFound = true;
        }
    }
}
bool checkProximity() {
    if (!bleInitialized) {
        BLEDevice::init("");
        delay(500);
        bleInitialized = true;
    } else {
    }
    
    BLEScan* scanner = BLEDevice::getScan();
    scanner->clearResults(); // Clear previous results
    
    ProximityScanner* callback = new ProximityScanner();
    scanner->setAdvertisedDeviceCallbacks(callback);
    scanner->setActiveScan(false);
    scanner->setInterval(100);
    scanner->setWindow(100);
    
    Serial.println("Searching...");
    BLEScanResults foundDevices = scanner->start(1, false);
    
    bool proximityDetected = callback->isBaseStationNear();
    delete callback;
    
    // DON'T deinit - keep BLE running
    return proximityDetected;
}