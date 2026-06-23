#include "../inc/drv_uart.h"
#include "../../platform/inc/comm_hal.h"

#define USART_BRR_DIV_Mantissa_Pos 4
void DRV_UART1_Init(uint32_t baudrate) {
	// Enable UART1 and GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN;

	// Config PA9 (TX) and PA10 (RX)
	GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9 | GPIO_CRH_MODE10 | GPIO_CRH_CNF10);
	GPIOA->CRH |= (GPIO_CRH_MODE9_0 | GPIO_CRH_MODE9_1 | GPIO_CRH_CNF9_1); // PA9, max speed = 50, AFO push-pull
	GPIOA->CRH |= GPIO_CRH_CNF10_0; // PA10, max speed = 50, Floating input

	uint32_t fclk = 72000000UL;

	// Config baudrate
	float usart_div = (float)fclk / (16.0f * (float)baudrate);
	uint16_t mantisa = (uint16_t)usart_div;
	uint8_t fraction = (uint8_t)(((usart_div - mantisa) * 16.0f) + 0.5f);

	if (fraction >= 16) {
		mantisa += 1;
		fraction = 0;
	}

	USART1->BRR = (mantisa << USART_BRR_DIV_Mantissa_Pos) | (fraction);

	// Enable UE, TE, RE, RXNEIE
	USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
	
	// Enable USART1 interrupt at NVIC
	NVIC_EnableIRQ(USART1_IRQn);
}

// IRQ Handler
void USART1_IRQHandler(void) {
	if (USART1->SR & USART_SR_RXNE) {
		uint8_t byte = (uint8_t)USART1->DR;
		CommHAL_RxByteCallback(byte);
	}
}
		
void DRV_UART1_SendByte(uint8_t data) {
	while (!(USART1->SR & USART_SR_TXE));
	USART1->DR = data;
}

void DRV_UART1_SendBytes(uint8_t *data, uint8_t len) {
	if (len == 0) return;
	for (uint8_t i = 0; i < len; i++) {
		DRV_UART1_SendByte(data[i]);
	}
}
