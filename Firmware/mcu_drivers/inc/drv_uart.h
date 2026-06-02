#ifndef DRV_UART_H_
#define DRV_UART_H_

#include "stm32f10x.h"


void DRV_UART1_Init(uint32_t baudrate);
void DRV_UART1_SendByte(uint8_t data);
void DRV_UART1_SendBytes(uint8_t *data, uint16_t len);
void DRV_UART1_SendString(char *str);
uint8_t DRV_UART1_Available(void);
uint8_t DRV_UART1_ReadByte(void);
#endif
