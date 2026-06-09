#ifndef COMM_HAL_H_
#define COMM_HAL_H_

#include <stdint.h>
#include "../../mcu_drivers/inc/drv_uart.h"

void CommHAL_Init(void);
void CommHAL_SendByte(uint8_t data);
void CommHAL_SendBytes(uint8_t *data, uint8_t len);
uint8_t CommHAL_Available(void);
uint8_t CommHAL_ReadByte(void);

#endif 

