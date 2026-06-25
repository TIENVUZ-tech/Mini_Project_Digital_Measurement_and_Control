#ifndef SENSOR_HAL_H
#define SENSOR_HAL_H

#include "stm32f10x.h"
#include "../../mcu_drivers/inc/drv_gpio.h"

#define DHT22_Port PA
#define DHT22_Pin 0

extern float Humidity;
extern float Temperature;
extern uint8_t DHT22_Data[5];

void DHT22_Start(Ports port, unsigned short pin);
int DHT22_Check_Response(Ports port, unsigned short pin);
uint8_t DHT22_Read_Byte(Ports port, unsigned short pin);
float ReadTemperature(void);
void DHT22_ReadData(void);

#endif
