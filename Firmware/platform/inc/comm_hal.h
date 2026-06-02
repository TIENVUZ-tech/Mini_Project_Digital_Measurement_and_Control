#ifndef COMM_HAL_H_
#define COMM_HAL_H_

#include <stdint.h>
#include "../../mcu_drivers/inc/drv_uart.h"

void CommHAL_Init(void);
void CommHAL_SendBytes(uint8_t *data, uint16_t len);
void CommHAL_SendString(char *str);
uint8_t CommHAL_Available(void);
uint8_t CommHAL_ReadByte(void);

#endif 

