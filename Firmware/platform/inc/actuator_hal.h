#ifndef __ACTUATOR_HAL_H
#define __ACTUATOR_HAL_H
#include <stdint.h>

typedef enum {
    ACTUATOR_FAN    = 0,
    ACTUATOR_HEATER = 1
} ActuatorDevice_t;

typedef enum {
    ACT_MODE_PWM   = 0,
    ACT_MODE_ONOFF = 1
} ActuatorMode_t;

typedef enum {
    UNIT_PERCENT,
    UNIT_RPM,
    UNIT_RPS,
    UNIT_DEG_C,
    UNIT_DEG_F,
    UNIT_DEG_K
} ActuatorUnit_t;


typedef struct {
    ActuatorDevice_t index;  
    ActuatorMode_t   mode;   
    ActuatorUnit_t   unit;   
    float            value;  
} ActuatorPrams_struct_t;

void ActuatorHAL_Init(void);
void ActuatorHAL_Set(ActuatorPrams_struct_t actuatorPrams);

#endif
