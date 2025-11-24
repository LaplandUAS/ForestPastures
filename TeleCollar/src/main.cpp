#include "main.h"

void setup() {
    // Remove interrupt pin setup - not needed anymore
    pinMode(SD_PIN, OUTPUT);
    digitalWrite(SD_PIN, HIGH);
    
    Serial.begin(115200);
    delay(100);

    // Command mode handling (unchanged)
    cmdMode = false;
    tStart = millis();
    debugln("Starting...");
    while (millis() - tStart <= MODE_TIMEOUT) {
        if (Serial.available()) {
            String iBuffer = Serial.readStringUntil('\n');
            iBuffer.trim();
            if (iBuffer == "CMD") cmdMode = true;
        }
    }
    
    if (cmdMode) Serial.printf("------COMMAND MODE------\n");
    while (cmdMode) {
        if (Serial.available()) {
            String cmd = Serial.readStringUntil('\n');
            cmd.trim();
            switch (hashCommand(cmd)) {
                case EXIT: cmdMode = false; Serial.printf("Returning to normal operation mode...\n"); break;
                case FLUSH: FlushCard(); break;
                case DELETE: ClearLog(); break;
                case PROX: CheckProx(); break;
                case TIME: Serial.printf("Unix time:%u\n",rtc.getEpoch()); break;
                default: Serial.printf("Invalid command: %s\n", cmd); break;
            }
        }
    }

    // SD card initialization (unchanged)
    if (!SD.begin()) {
        debug("Card mount failed.");
    }
    
    rtc.setTime(storedEpoch+SLEEP_CHECK_INTERVAL);
    SetRTCReference(&rtc);
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    debug("SD Card Size: %lluMB\n", cardSize);
    
    if (SD.exists("/mx0.log")) {
        debug("Log found!\n");
    } else {
        debug("Log not found! Creating new...\n");
        logFile = SD.open("/mx0.log", FILE_WRITE);
        logFile.close();
    }
    
    delay(1000);
    SD.end();
    digitalWrite(SD_PIN, LOW);

    // Sensor initialization with software-only approach
    Wire.begin();
    if (!acc.begin()) {
        debugln("Sensor init failed!");
    } else {
        debugln("Sensor OK!");
    }

    // Check if waking from deep sleep
    handleWakeup();
    
    // Initialize motion tracking
    lastMotionTime = millis();
    currentState = STATE_ACTIVE;
    
    // Get initial reading
    while (!acc.dataReady()) delay(10);
    lastReading = acc.GetMovement();
    
    debugln("System ready - starting motion detection");
}

void loop() {
    static unsigned long lastSampleTime = 0;
    unsigned long currentTime = millis();
    
    // Rate limit sampling to 1Hz for battery life
    if (currentTime - lastSampleTime < 1000) {
        delay(50);
        return;
    }
    lastSampleTime = currentTime;
    
    // Get current sensor reading
    v3Di currentReading = acc.GetMovement();
    
    switch (currentState) {
        case STATE_ACTIVE:
        case STATE_SETTLING: {
            // Check for motion (compare with previous reading)
            bool hasMotion = checkForMotion(currentReading, lastReading);
            
            if (hasMotion) {
                if (currentState == STATE_SETTLING) {
                    debugln("Motion detected - staying active");
                }
                currentState = STATE_ACTIVE;
                lastMotionTime = currentTime;
                noMotionCount = 0; // Reset counter
                
                // Log the data
                logData(currentReading);
                
                debug("Motion: X:%d Y:%d Z:%d (Δ: %d,%d,%d)\n",
                    currentReading.x, currentReading.y, currentReading.z,
                    abs(currentReading.x - lastReading.x),
                    abs(currentReading.y - lastReading.y),
                    abs(currentReading.z - lastReading.z));
            } else {
                // No motion detected - increment counter
                noMotionCount++;
                
                if (currentState == STATE_ACTIVE && noMotionCount == 1) {
                    debug("No motion - entering settling phase (need %d samples)\n", MOTION_SETTLE_SAMPLES);
                    currentState = STATE_SETTLING;
                }
                
                // Check if we should go to sleep
                if (noMotionCount >= MOTION_SETTLE_SAMPLES) {
                    sleepBaseline = currentReading;
                    enterDeepSleep();
                }
                
                debug(".(%d)", noMotionCount);  // Show settling progress
            }
            break;
        }
        
        case STATE_SLEEPING:
            // This should never happen in normal operation
            debugln("Error: In sleep state but loop is running");
            break;
    }
    
    lastReading = currentReading;
}

bool checkForMotion(v3Di current, v3Di baseline) {
    int deltaX = abs(current.x - baseline.x);
    int deltaY = abs(current.y - baseline.y);
    int deltaZ = abs(current.z - baseline.z);
    
    return (deltaX > WAKE_THRESHOLD || 
            deltaY > WAKE_THRESHOLD || 
            deltaZ > WAKE_THRESHOLD);
}

void enterDeepSleep() {
    debug("\n=== ENTERING DEEP SLEEP ===\n");
    debug("Sleep baseline: X:%d Y:%d Z:%d\n", 
                  sleepBaseline.x, sleepBaseline.y, sleepBaseline.z);
    debug("Will wake in %d seconds to check for motion\n", SLEEP_CHECK_INTERVAL);
    
    // Finalize any pending operations
    onReadingCycleComplete();
    
    // Set system state and mark baseline as valid
    currentState = STATE_SLEEPING;
    hasValidBaseline = true;  // This thing will survive deep sleep
    
    Serial.flush(); // Ensure all serial data is sent
    storedEpoch = rtc.getEpoch();
    // Configure timer wake-up
    esp_sleep_enable_timer_wakeup(SLEEP_CHECK_INTERVAL * 1000000ULL);
    
    // Enter deep sleep - system will restart from setup() on wake
    esp_deep_sleep_start();
}

void handleWakeup() {
    esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
    
    if (wakeup_cause == ESP_SLEEP_WAKEUP_TIMER) {
        debugln("\n=== WOKE FROM DEEP SLEEP ===");
        debugln("Checking for motion...");
        // Check if we have a valid baseline from before sleep
        if (!hasValidBaseline) {
            debugln("No valid baseline - treating as normal boot");
            return;
        }
        if((rtc.getEpoch() - lastTransferTime) > minTransferWait)
        {
            lastTransferTime = rtc.getEpoch();
            CheckProx();
        }
        else
        Serial.printf("Still too fresh... (%u)\n", rtc.getEpoch());
        // Wait for sensor to be ready
        delay(100);
        while (!acc.dataReady()) delay(10);
        
        v3Di currentReading = acc.GetMovement();
        
        if (checkForMotion(currentReading, sleepBaseline)) {
            debug("MOTION DETECTED! Current: X:%d Y:%d Z:%d vs Sleep baseline: X:%d Y:%d Z:%d\n",
                          currentReading.x, currentReading.y, currentReading.z,
                          sleepBaseline.x, sleepBaseline.y, sleepBaseline.z);
            
            // Motion detected - stay awake
            currentState = STATE_ACTIVE;
            lastMotionTime = millis();
            hasValidBaseline = false; // Reset for next sleep cycle
            return;
        } else {
            debug("Still stationary (X:%d Y:%d Z:%d) - returning to sleep\n",
                          currentReading.x, currentReading.y, currentReading.z);
            
            // Update baseline to handle slow drift
            sleepBaseline = currentReading;
            
            // Go back to sleep immediately
            Serial.flush();
            storedEpoch = rtc.getEpoch();
            esp_sleep_enable_timer_wakeup(SLEEP_CHECK_INTERVAL * 1000000ULL);
            esp_deep_sleep_start();
        }
    } else if (wakeup_cause == ESP_SLEEP_WAKEUP_UNDEFINED) {
        debugln("Normal boot (not waking from sleep)");
        hasValidBaseline = false; // Ensure clean state on normal boot
    }
}

void logData(v3Di movement) {
    if (bufIndex >= MAX_READ) {
        writeFullBuffer();
    } else {
        sprintf(rwBuf, "%lu:%i:%i:%i", rtc.getEpoch(), movement.x, movement.y, movement.z);
        strcpy(saveBuffer[bufIndex], rwBuf);
        bufIndex++;
    }
}

void onReadingCycleComplete() {
    if (bufIndex > 0) {
        writeFullBuffer();
    }
}

void writeFullBuffer() {
    if (bufIndex == 0) return;
    
    digitalWrite(SD_PIN, HIGH);
    SD.begin();
    delay(100); // Reduced delay
    
    debug("\nWriting %i entries to SD...\n", bufIndex);
    logFile = SD.open("/mx0.log", FILE_APPEND);
    
    for (int i = 0; i < bufIndex; i++) {
        logFile.println(saveBuffer[i]);
        debugln(saveBuffer[i]);
    }
    
    logFile.close();
    bufIndex = 0;
    SD.end();
    digitalWrite(SD_PIN, LOW);
    debugln("Write complete");
}

// Command handling functions (unchanged)
CmdCode hashCommand(String& inString) {
    if (inString == "EXIT") return EXIT;
    if (inString == "FLUSH") return FLUSH;
    if (inString == "DELETE") return DELETE;
    if (inString == "PROX") return PROX;
    if (inString == "TIME")return TIME;
    return INVALID;
}

void FlushCard() {
    digitalWrite(SD_PIN, HIGH);
    if (!SD.begin()) {
        Serial.printf("Card mount failed.\n");
        digitalWrite(SD_PIN, LOW);
        return;
    }
    if (SD.exists("/mx0.log")) {
        Serial.printf("Log found... Preparing dump\n");
        logFile = SD.open("/mx0.log", FILE_READ);
        while (logFile.available()) {
            Serial.write(logFile.read());
        }
        logFile.close();
        Serial.printf("Log dump done!\n");
    }
    SD.end();
    digitalWrite(SD_PIN, LOW);
}

void ClearLog() {
    digitalWrite(SD_PIN, HIGH);
    if (!SD.begin()) {
        Serial.printf("Card mount failed.\n");
        digitalWrite(SD_PIN, LOW);
        return;
    }
    if (SD.exists("/mx0.log")) {
        Serial.printf("Log found... Removing...\n");
        if (SD.remove("/mx0.log")) {
            Serial.printf("File deleted.\n");
        } else {
            Serial.printf("Delete failed.\n");
        }
    }
    SD.end();
    digitalWrite(SD_PIN, LOW);
}
void CheckProx()
{
    Serial.printf("Checking proximity...\n");
    
    // Set a watchdog timer as backup
    unsigned long startTime = millis();
    const unsigned long timeout = 10000; // 10 second timeout
    
    bool result = false;
    bool completed = false;
    
    // Simple timeout wrapper
    while (!completed && (millis() - startTime < timeout)) {
        result = checkProximity();
        completed = true;
    }
    
    if (!completed) {
        Serial.println("Proximity check timed out!");
        // Force BLE cleanup if stuck
        BLEDevice::deinit(true);
        return;
    }
    
    if (result) {
        Serial.printf("In range! Initiating ESP-NOW communication...\n");
        
        // Initialize ESP-NOW link
        BeginLink();
        delay(500); // Give ESP-NOW time to initialize

        if (TransferLogFile()) {
            Serial.println("Log file transfer completed successfully!");
            
            // Wait for time sync response (your existing code)
            Serial.println("Waiting for time synchronization...");
            unsigned long responseStart = millis();
            
            while (!ResponseReceived() && (millis() - responseStart < 3000)) {
                delay(50);
            }
            
            if (ResponseReceived()) {
                Serial.println("Time synchronization completed!");
                Serial.printf("Device time is now synced to: %lu\n", rtc.getEpoch());
            }
            
        } else {
            Serial.println("Log file transfer failed");
        }
        
    } else {
        Serial.printf("Out of range!\n");
    }
}

bool TransferLogFile()
{
    digitalWrite(SD_PIN, HIGH);

    if (!SD.begin()) {
        Serial.printf("Card mount failed.\n");
        digitalWrite(SD_PIN, LOW);
        return false;
    }

    if (!SD.exists("/mx0.log")) {
        Serial.println("Nothing to send!");
        digitalWrite(SD_PIN, LOW);
        return false;
    }
    
    Serial.printf("Log found... Preparing transmission\n");
    logFile = SD.open("/mx0.log", FILE_READ);
    
    if (!logFile) {
        Serial.println("Failed to open log file");
        digitalWrite(SD_PIN, LOW);
        return false;
    }
    
    struct_message msg;
    int32_t lineNumber = 0;
    bool allLinesSuccessful = true;
    
    // Read and send file line by line
    while (logFile.available()) {
        String line = logFile.readStringUntil('\n');
        line.trim(); // Remove whitespace and newlines
        
        // Skip empty lines
        if (line.length() == 0) {
            continue;
        }
        
        // Prepare message
        msg.type = (pkType)0; // Custom type, ignore for now
        msg.inf = lineNumber;
        
        // Copy line to data buffer (truncate if too long)
        strncpy(msg.data, line.c_str(), sizeof(msg.data) - 1);
        msg.data[sizeof(msg.data) - 1] = '\0'; // Ensure null termination
        
        // Send this line
        Serial.printf("Sending line %d: %.20s%s\n", 
                     lineNumber, 
                     msg.data, 
                     strlen(msg.data) > 20 ? "..." : "");
        
        if (SendData(msg)) {
            // Wait for this line to be sent successfully
            unsigned long sendStart = millis();
            const unsigned long sendTimeout = 3000; // 3 second timeout per line
            
            while (!IsDataTransferComplete() && (millis() - sendStart < sendTimeout)) {
                delay(50); // Shorter delay for more responsive checking
            }
            
            if (IsDataTransferComplete()) {
                Serial.printf("Line %d sent successfully\n", lineNumber);
                lineNumber++;
            } else {
                Serial.printf("Line %d timed out\n", lineNumber);
                allLinesSuccessful = false;
                break; // Stop on first failure, TODO: maybe retry?
            }
            
        } else {
            Serial.printf("Failed to initiate transmission of line %d\n", lineNumber);
            allLinesSuccessful = false;
            break;
        }
        
        // Small delay between lines to avoid overwhelming the receiver
        delay(100);
    }
    
    logFile.close();
    
    // Send end-of-transmission marker
    if (allLinesSuccessful) {
        Serial.println("Sending end-of-transmission marker...");
        
        msg.type = (pkType)0;
        msg.inf = -1; // End of transmission marker
        strcpy(msg.data, "END_OF_LOG");
        
        if (SendData(msg)) {
            unsigned long sendStart = millis();
            while (!IsDataTransferComplete() && (millis() - sendStart < 3000)) {
                delay(50);
            }
            
            if (IsDataTransferComplete()) {
                Serial.printf("Log transmission complete! Sent %d lines\n", lineNumber);
                
                ClearLog();
                digitalWrite(SD_PIN, LOW);
                return true;
            }
        }
        
        Serial.println("Failed to send end-of-transmission marker");
    }
    
    Serial.printf("Log transmission failed after %d lines\n", lineNumber);
    digitalWrite(SD_PIN, LOW);
    return false;
}