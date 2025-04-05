#ifndef __WIFI_MODULE_H__
#define __WIFI_MODULE_H__

#include <Arduino.h>
#include <WiFi.h>

#define WIFI_SSID           "KATANA"
#define WIFI_PASSWORD       "030323akpt"


void initWifi(void* ptrParameter);
void taskCheckWifiConnection(void* ptrParameter);


#endif

