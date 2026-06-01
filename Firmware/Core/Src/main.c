#include "stm32f10x.h"
#include ".\Firmware\mcu_drivers\src\drv_uart.h"

int main() {
	DRV_UART1_Init(115200);
	DRV_UART1_SendString("Hello world\r\n");
	uint8_t data = 0;
	while(1) {
		if (DRV_UART1_Available()) {
			data = DRV_UART1_ReadByte();
			DRV_UART1_SendByte(data);
		}
	}
}
		