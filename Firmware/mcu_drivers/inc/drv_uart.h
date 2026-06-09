#ifndef DRV_UART_H_
#define DRV_UART_H_

#include "stm32f10x.h"

#define UART_RX_BUFFER_SIZE 64

// Initialize UART with baudrate parameter.
void DRV_UART1_Init(uint32_t baudrate);

// Send a byte
void DRV_UART1_SendByte(uint8_t data);

// Send multiple bytes
void DRV_UART1_SendBytes(uint8_t *data, uint8_t len);

// Return the number of bytes
uint8_t DRV_UART1_Available(void);

// Read a byte
uint8_t DRV_UART1_ReadByte(void);
#endif
