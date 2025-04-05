#ifndef __SENSOR_H__
#define __SENSOR_H__


#include <Arduino_MQTT_Client.h>
#include <WiFi.h>
#include <DHT20.h>
#include <ThingsBoard.h>

// define the position of pin in I2C protocol
#define SDA_PIN GPIO_NUM_11
#define SCL_PIN GPIO_NUM_12

// define the thingsboard information
#define THINGSBOARD_SERVER  "app.coreiot.io"
#define THINGSBOARD_PORT    1883U


// define the token of the device
#define TOKEN_DEVICE        "0Lyh0XOhQAKBujdConR7"
#define MAX_MESSAGE_SIZE    1024U

void taskReadSensor(void* ptrParameter);

#endif