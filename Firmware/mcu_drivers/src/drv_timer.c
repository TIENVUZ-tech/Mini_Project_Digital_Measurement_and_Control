#include "stm32f10x.h"
#include "drv_timer.h"

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
