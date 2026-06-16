#ifndef DRV_TIMER_H
#define DRV_TIMER_H

#include "stm32f10x.h"

typedef enum{
	T, T1, T2, T3, T4
} Timers;

void timerInitForDelay(void);
void SystickInit(void);
void delay_us(int us);
void delay_ms(int ms);
/*
void TIM2_Init(void);
uint32_t DRV_TIM_GetMs(void);
*/
#endif
