#include "stm32f10x.h"
#include "../inc/drv_timer.h"

volatile uint32_t tick = 0;

void timerInitForDelay(void){
	//Su dung TIM4 de tao ham delay
	RCC->APB1ENR |= 1 << 2;
	TIM4->PSC = 72 - 1;
	TIM4->EGR |= 1 << 0;
	TIM4->CR1 |= 1 << 0;
}

void SystickInit(void){
	SysTick->LOAD = 72000 - 1;
	SysTick->VAL = 0;
	SysTick->CTRL |=  (1 << 2) | (1 << 1) | (1 << 0);
}


void delay_us(int us){
	TIM4->ARR = us - 1;
	TIM4->CNT = 0;
	TIM4->SR &= ~(1 << 0);
	while(!(TIM4->SR & 1 << 0));
	TIM4->SR |= 1 << 0;
}

void delay_ms(int ms){
	for(int i = 0; i < ms; i++){
		delay_us(1000);
	}
}


/*
void TIM2_Init(void) {
    RCC->APB1ENR |= (1 << 0);   // ? thêm dòng này

    TIM2->PSC = 72 - 1;         // 72MHz / 72 = 1MHz
    TIM2->ARR = 1000 - 1;       // 1MHz / 1000 = 1ms per tick
    TIM2->CNT = 0;
    TIM2->SR  &= ~(1 << 0);     // xóa UIF tru?c khi b?t
    TIM2->DIER |= (1 << 0);     // UIE
    TIM2->CR1  |= (1 << 0);     // CEN - start timer
    NVIC_EnableIRQ(TIM2_IRQn);
}

static volatile uint32_t s_tick_ms = 0;

void TIM2_IRQHandler(void) {
    if (TIM2->SR & (1 << 0)) {
        TIM2->SR &= ~(1 << 0);
        s_tick_ms++;
    }
}

uint32_t DRV_TIM_GetMs(void) {
    return s_tick_ms;
}
*/
