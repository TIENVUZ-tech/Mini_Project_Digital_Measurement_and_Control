#include "../inc/comm_hal.h"

#define baudrate 115200
void CommHAL_Init(void){
	DRV_UART1_Init(baudrate);
}

void CommHAL_SendBytes(uint8_t *data, uint16_t len) {
	DRV_UART1_SendBytes(data, len);
}

void CommHAL_SendString(char *str) {
	DRV_UART1_SendString(str);
}

uint8_t CommHAL_Available(void) {
	return DRV_UART1_Available();
}

uint8_t CommHAL_ReadByte(void) {
	return DRV_UART1_ReadByte();
}
