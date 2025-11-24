#ifndef STORAGE_H
#define STORAGE_H
#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

class MassStorage
{
    private:
        File m_logFile;

    public:
        bool InitCard();
        void MakeFile(const char *path);
        void RemvFile(const char *path);
        void WriteFile(const char *path, const char *data);
        void AppendFile(const char *path, const char *data);
};

#endif