#ifndef SENSOR_APP_H
#define SENSOR_APP_H

#include "stm32f10x.h"
#include "soft_timer.h"
#include "sensor_hal.h"
#include "drv_timer.h"

#define DHT22_Port PA
#define DHT22_Pin 0

SoftTimer_t DHT22_timer;
SoftTimer_t led_timer;
uint8_t DHT22_Data[5];

float Humidity;
float Temperature;

void DHT22_task(void);
float readHumidity(void);
float readTemperature(void);

#endif
