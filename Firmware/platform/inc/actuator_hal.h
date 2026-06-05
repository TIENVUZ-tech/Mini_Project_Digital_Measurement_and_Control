#ifndef __ACTUATOR_HAL_H
#define __ACTUATOR_HAL_H

#include <stdint.h>

void ActuatorHAL_Init(void);

void ActuatorHAL_SetFan(uint8_t duty_percent);

void ActuatorHAL_SetLight(uint8_t intensity_percent);

#endif
