#include "storage.h"

bool MassStorage::InitCard()
{
    if (!SD.begin(5)) {
        Serial.printf("Storage mount fail\n");
        return false;
    }
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.printf("No storage attached\n");
        return false;
    }
    return true;
}

void MassStorage::MakeFile(const char *path)
{
    Serial.printf("Creating log file:%s\n", path);
    m_logFile = SD.open(path, FILE_WRITE);
    if (m_logFile) {
        Serial.printf("Created log file:%s\n", path);
        m_logFile.close();
    } else {
        Serial.printf("Failed to create log file\n");
    }
}

void MassStorage::RemvFile(const char *path)
{
    Serial.printf("Deleting file: %s\n", path);
    if (SD.remove(path)) {
        Serial.printf("File deleted\n");
    } else {
        Serial.printf("Delete failed\n");
    }
}

void MassStorage::WriteFile(const char *path, const char *data)
{
    Serial.printf("Writing file: %s\n", path);

    m_logFile = SD.open(path, FILE_WRITE);
    if (!m_logFile) {
        Serial.printf("File failed to open\n");
        return;
    }
    if (m_logFile.print(data)) {
        Serial.printf("File written\n");
    } else {
        Serial.printf("Write failed\n");
    }
    m_logFile.close();
}

void MassStorage::AppendFile(const char *path, const char *data)
{
    Serial.printf("Appending to file: %s\n", path);

    m_logFile = SD.open(path, FILE_APPEND);
    if (!m_logFile) {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (m_logFile.print(data)) {
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    m_logFile.close();
}