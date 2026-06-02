#ifndef DRV_TIMER_H
#define DRV_TIMER_H

#include "stm32f10x.h"

typedef enum{
	T, T1, T2, T3, T4
} Timers;

void Delay_us(Timers timer, int us);
void Delay_ms(Timers timer, int ms);
void delay_us(int us);
void delay_ms(int us);

#endif
