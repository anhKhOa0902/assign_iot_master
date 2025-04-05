#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <DHT20.h>
#define SDA_PIN GPIO_NUM_11
#define SCL_PIN GPIO_NUM_12

void TaskTemperatureHumidity(void* ptrParameter);

#endif