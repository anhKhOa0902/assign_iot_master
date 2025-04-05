#include "../include/sensor.h"

void TaskTemperatureHumidity(void* ptrParameter)
{
  DHT20 dht;
  Wire.begin(SDA_PIN, SCL_PIN);
  dht.begin();
  while (true)
  {
    dht.read();
    double temperature = dht.getTemperature();
    double humidity = dht.getHumidity();

    Serial.println("Temp: " + String(temperature) + " *C");
    Serial.println("Humidity: " + String(humidity) +  " %");
    Serial.println("\n \n");
    
    vTaskDelay(3000);
  }
}