#ifndef SENSOR_HAL_H
#define SENSOR_HAL_H

#include "stm32f10x.h"
#include "drv_gpio.h"

void DHT22_Start(Ports port, unsigned short pin);
int DHT22_Check_Response(Ports port, unsigned short pin);
uint8_t DHT22_Read_Byte(Ports port, unsigned short pin);

#endif
