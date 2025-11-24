#ifndef NETTIME_H
#define NETTIME_H
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

inline uint8_t g_attempts = 32;

inline const char* g_ssid     = "myssid";
inline const char* g_pwd = "mypwd";

inline const char* g_ntpURL = "pool.ntp.org";
inline const long  g_gmtOffset = 0; //DON'T SET!!!!
inline const int   g_dsavings = 0;  //DON'T SET!!!!

void UpdateTime();

#endif