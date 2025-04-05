#include "../include/blinkLed.h"

void taskLEDControl(void* ptrParameter)
{
  pinMode(GPIO_LED, OUTPUT);
  int ledState = 0;
  while (true)
  {
    digitalWrite(GPIO_LED, ledState);
    ledState  = !ledState;
    vTaskDelay(2000);
  }
}