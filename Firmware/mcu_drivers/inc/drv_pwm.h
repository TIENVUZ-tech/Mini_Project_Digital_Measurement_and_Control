#ifndef DRV_PWM_H_
#define DRV_PWM_H_

#include <stdint.h> 



void DRV_PWM_Init(void);


void DRV_PWM_Start(uint8_t channel);


void DRV_PWM_SetDuty(uint8_t channel, uint16_t duty);


void DRV_PWM_Stop(uint8_t channel);

#endif 
