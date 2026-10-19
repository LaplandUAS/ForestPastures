#include "nettime.h"

void UpdateTime()
{
    Serial.printf("Connecting to:%s\n",g_ssid );
    WiFi.begin(g_ssid,g_pwd);
    while(WiFi.status()!=WL_CONNECTED && (g_attempts--)>0)
    {
        delay(500);
        Serial.printf(".");
    }
    if(g_attempts == 0) {Serial.printf("\nFailed to connect to:%s, using rtc memory...\n",g_ssid); return;}
    Serial.printf("Connection succesful!\n");
    g_attempts = 8;
    configTime(g_gmtOffset, g_dsavings, g_ntpURL);
    delay(2000);
    time_t now = 0;
    while(now<100000&&(g_attempts--)>0)
    {
        delay(500);
        time(&now);
    }
    uint32_t unix_epoch = (uint32_t)now;

    Serial.printf("UNIX epoch: %u\n", unix_epoch);
    //WiFi.disconnect(true);
    //WiFi.mode(WIFI_OFF);
}