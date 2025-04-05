#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

void initOLED();
void displaySensorData(float temperature, float humidity);
void displaySensorData(float temperature, float humidity, int soilMoisture);

#endif