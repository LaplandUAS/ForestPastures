#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <SparkFunADXL313.h>
#include "debugtypes.h"

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} v3Di;

#define INT_PIN         13
#define WAKE_THRESHOLD  10
#define MOTION_SETTLE_SAMPLES 3  // Number of consecutive no-motion samples before sleep
#define SLEEP_CHECK_INTERVAL 30  // Seconds between motion checks during sleep

class Sensor {
    private:
        ADXL313 m_sensor;
    public:
        Sensor();
        bool begin();
        v3Di GetMovement();
        bool dataReady();
};

#endif