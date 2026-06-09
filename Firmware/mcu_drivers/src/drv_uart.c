#include "../inc/drv_uart.h"

static volatile uint8_t rx_buf[UART_RX_BUFFER_SIZE];
static volatile uint8_t rx_head = 0; // IRQ write from here
static volatile uint8_t rx_tail = 0; // App read from here

void DRV_UART1_Init(uint32_t baudrate) {
	// Enable UART1 and GPIOA
	RCC->APB2ENR |= (1 << 14) | (1 << 2);

	// Config PA9 (TX) and PA10 (RX)
	GPIOA->CRH &= ~(0xFF << 4);
	GPIOA->CRH |= 0xB << 4; // PA9, max speed = 50, AFO push-pull
	GPIOA->CRH |= 0x4 << 8; // PA10, max speed = 50, Floating input

	uint32_t fclk = 72e6;

	// Config baudrate
	float usart_div = (float)fclk / (16.0f * (float)baudrate);
	uint16_t mantisa = (uint16_t)usart_div;
	uint8_t fraction = (uint8_t)(((usart_div - mantisa) * 16.0f) + 0.5f);

	if (fraction >= 16) {
		mantisa += 1;
		fraction = 0;
	}

	USART1->BRR = (mantisa << 4) | (fraction);

	// Enable UE, TE, RE, RXNEIE
	USART1->CR1 |= (1 << 13) | (1 << 3) | (1 << 2) | (1 << 5);
	
	// Enable USART1 interrupt at NVIC
	NVIC_EnableIRQ(USART1_IRQn);
}

// IRQ Handler
void USART1_IRQHandler(void) {
	if (USART1->SR & (1 << 5)) {
		uint8_t byte = USART1->DR;
		uint8_t next_head = (rx_head + 1) & (UART_RX_BUFFER_SIZE - 1);
		if (next_head != rx_tail) {
			rx_buf[rx_head] = byte;
			rx_head = next_head;
		}
	}
}
		
void DRV_UART1_SendByte(uint8_t data) {
	while (!(USART1->SR & (1 << 7)));
	USART1->DR = data;
}

void DRV_UART1_SendBytes(uint8_t *data, uint8_t len) {
	if (len == 0) return;
	for (uint8_t i = 0; i < len; i++) {
		DRV_UART1_SendByte(data[i]);
	}
}

uint8_t DRV_UART1_Available(void) {
	return (uint8_t)((rx_head - rx_tail) & (UART_RX_BUFFER_SIZE - 1));
}

uint8_t DRV_UART1_ReadByte(void) {
	if (rx_head == rx_tail) return 0;
	
	uint8_t byte = rx_buf[rx_tail];
	rx_tail = (rx_tail + 1) & (UART_RX_BUFFER_SIZE - 1);
	return byte;
}

