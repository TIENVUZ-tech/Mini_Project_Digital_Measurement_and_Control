#include "drv_uart.h"

void DRV_UART1_Init(uint32_t baudrate) {
	// Enable UART1 and GPIOA
	RCC->APB2ENR |= (1 << 14) | (1 << 2);

	// Config PA9 (TX) and PA10 (RX)
	GPIOA->CRH &= ~(0xFF << 4);
	GPIOA->CRH |= 0xB << 4; // PA9, max speed = 50, AFO push-pull
	GPIOA->CRH |= 0x4 << 8; // PA10, max speed = 50, Floating input

	uint32_t fclk = 72e6;

	// Config baudrate
	float usart_div = (float)fclk / (16.0f * baudrate);
	uint16_t mantisa = (uint16_t)usart_div;
	uint8_t fraction = (uint8_t)(((usart_div - mantisa) * 16.0f) + 0.5f);

	if (fraction >= 16) {
		mantisa += 1;
		fraction = 0;
	}

	USART1->BRR = (mantisa << 4) | (fraction);

	// Enable USART
	USART1->CR1 |= (1 << 13) | (1 << 3) | (1 << 2);
}

void DRV_UART1_SendByte(uint8_t data) {
	while (!(USART1->SR & (1 << 7)));
	USART1->DR = data;
}

void DRV_UART1_SendBytes(uint8_t *data, uint16_t len) {
	if (len == 0) return;
	for (uint16_t i = 0; i < len; i++) {
		DRV_UART1_SendByte(data[i]);
	}
}

void DRV_UART1_SendString(char *str) {
	while (*str) {
		DRV_UART1_SendByte(*str++);
	}
}

uint8_t DRV_UART1_Available(void) {
	if (USART1->SR & (1 << 5)) {
		return 1;
	}
	return 0;
}
uint8_t DRV_UART1_ReadByte(void) {
	return USART1->DR;
}

