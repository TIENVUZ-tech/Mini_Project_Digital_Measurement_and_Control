#ifndef DRV_UART_H_
#define DRV_UART_H_

#include "stm32f10x.h"

// Initialize UART with baudrate parameter.
void DRV_UART1_Init(uint32_t baudrate);

// Send a byte
void DRV_UART1_SendByte(uint8_t data);

// Send multiple bytes
void DRV_UART1_SendBytes(uint8_t *data, uint8_t len);
#endif
