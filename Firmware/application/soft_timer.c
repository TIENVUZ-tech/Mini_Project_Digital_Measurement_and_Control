#include "drv_timer.h"
#include "soft_timer.h"

void SoftTimer_Start(SoftTimer_t *timer, uint32_t period_ms, TimerCallback_t callback){
	timer->counter = period_ms;
	timer->period = period_ms;
	timer->enable = 1;
	timer->event = 0;
	timer->callback = callback;
}

void SoftTimer_Stop(SoftTimer_t *timer){
	timer->enable = 0;
}

void SoftTimer_Update(SoftTimer_t *timer){
	if(timer->enable){
		if(timer->counter > 0){
			timer->counter--;		}
		if(timer->counter == 0){
			if(timer->event < 255){
				timer->event++;
			}
			timer->counter = timer->period;
		}
	}
}

void SoftTimer_Dispatch(SoftTimer_t *timer){
	while(timer->event){
		timer->event--;
		if(timer->callback){
			timer->callback();
		}
	}
}

