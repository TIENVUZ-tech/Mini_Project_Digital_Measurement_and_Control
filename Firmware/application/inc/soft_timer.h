#ifndef SOFT_TIMER_H
#define SOFT_TIMER_H

#include "stm32f10x.h"
#include "../../mcu_drivers/inc/drv_timer.h"

typedef void (*TimerCallback_t)(void);

typedef struct{
	volatile uint32_t counter;
	volatile uint32_t period;
	volatile uint32_t event;
	volatile uint8_t enable;
	TimerCallback_t callback;
} SoftTimer_t;


void SoftTimer_Start(SoftTimer_t *timer, uint32_t period_ms, TimerCallback_t callback);
void SoftTimer_Stop(SoftTimer_t *timer);
void SoftTimer_Update(SoftTimer_t *timer);
void SoftTimer_Dispatch(SoftTimer_t *timer);

#endif

