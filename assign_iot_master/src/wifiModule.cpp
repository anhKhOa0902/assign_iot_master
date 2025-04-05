#include "../include/wifiModule.h"
#include <ThingsBoard.h>

void initWifi(void* ptrParameter) 
{
  Serial.println("Connecting to AP ...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

void taskCheckWifiConnection(void* ptrParameter) 
{
  Serial.println("Connecting to AP ...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (true)
  {
    if (WiFi.status() != WL_CONNECTED)
      initWifi(NULL);
    else
      Serial.println("Connected to AP");
    vTaskDelay(10000);
  }
}