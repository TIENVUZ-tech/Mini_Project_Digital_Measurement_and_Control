#include "stm32f10x.h"
#include "../inc/drv_timer.h"

void Delay_us(Timers timer, int us){
	int prescaler = 72 - 1;
	TIM_TypeDef *tim;

  //Choose timer
	if(timer == 1) RCC->APB2ENR |= 1 << 11;
	else RCC->APB1ENR |= 1 << (timer - 2);
	
	switch(timer){
		case 1: tim = TIM1;
			break;
		case 2: tim = TIM2;
			break;
		case 3: tim = TIM3;
			break;
		case 4: tim = TIM4;
			break;
		default: return;
	}

  //Set up for timer
	tim->PSC = prescaler;
	tim->ARR = us - 1;
	tim->CNT = 0;
	tim->SR &= ~(1 << 0);
	tim->CR1 |= 1 << 0;
	while(!(tim->SR & 1 << 0));
	tim->CR1 &= ~(1 << 0);
	tim->SR &= ~(1 << 0);
}

void Delay_ms(Timers timer, int ms){
	for(int i = 0; i < ms; i++)
		Delay_us(timer, 1000);
}

void delay_us(int us){
	Delay_us(T4, us);
}

void delay_ms(int ms){
	Delay_ms(T4, ms);
}

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