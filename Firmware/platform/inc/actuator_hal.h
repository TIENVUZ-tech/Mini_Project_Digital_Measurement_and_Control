#ifndef __ACTUATOR_HAL_H
#define __ACTUATOR_HAL_H

#include <stdint.h>

typedef enum {
    UNIT_PERCENT,
    UNIT_RPM,
    UNIT_RPS,
    UNIT_Do_C,    
    UNIT_Do_F,
    UNIT_Do_K,     
} ActuatorType_t;

void ActuatorHAL_Init(void);

void ActuatorHAL_SetFan(ActuatorType_t type, float value);
void ActuatorHAL_SetHeater(ActuatorType_t type, float value);

#endif
