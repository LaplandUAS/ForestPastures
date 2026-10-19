#ifndef NETTIME_H
#define NETTIME_H
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

inline uint8_t g_attempts = 32;

//inline const char* g_ssid     = "CharmRasia";
//inline const char* g_pwd = "#!R/PMF?PMJ6";
inline const char* g_ssid     = "KalyRasia";
inline const char* g_pwd = "Ammu31P0mm!a";

inline const char* g_ntpURL = "pool.ntp.org";
inline const long  g_gmtOffset = 0; //ÄLÄ ASETA
inline const int   g_dsavings = 0;  //ÄLÄ ASETA

void UpdateTime();

#endif