#include "sensor.h"

Sensor::Sensor() {}

bool Sensor::begin()
{
    if (!m_sensor.begin()) {
        return false;
    }
    
    m_sensor.setRange(ADXL313_RANGE_4_G);
    
    // DISABLE all hardware interrupts - pure software control. TODO: Cleanup remaining interrupt stuff in main
    m_sensor.ActivityINT(false);
    m_sensor.InactivityINT(false);
    m_sensor.DataReadyINT(false);
    
    // Keep in continuous measurement mode
    m_sensor.measureModeOn();
    
    debugln("Sensor initialized in software-only mode for maximum battery life");
    return true;
}

v3Di Sensor::GetMovement()
{
    if (m_sensor.dataReady()) {
        m_sensor.readAccel();
        return (v3Di){m_sensor.x, m_sensor.y, m_sensor.z};
    }
    return (v3Di){0, 0, 0};
}

bool Sensor::dataReady()
{
    return m_sensor.dataReady();
}