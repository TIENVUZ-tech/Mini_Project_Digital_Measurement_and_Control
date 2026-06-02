#ifndef DRV_TIMER_H
#define DRV_TIMER_H

#include "stm32f10x.h"

enum timers{
	T, T1, T2, T3, T4
};

void Delay_us(unsigned short timer, int us);
void Delay_ms(unsigned short timer, int ms);
void delay_us(int us);
void delay_ms(int us);

#endif
