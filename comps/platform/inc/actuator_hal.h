#ifndef __ACTUATOR_HAL_H
#define __ACTUATOR_HAL_H
#include <stdint.h>

typedef enum {
    ACTUATOR_FAN    = 0,
    ACTUATOR_HEATER = 1,
	  ACTUATOR_MICRO  = 2,
	  ACTUATOR_LIGHT  = 3
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
    UNIT_DEG_K,
	  UNIT_LUX
} ActuatorUnit_t;


typedef struct {
    ActuatorDevice_t index;  
    ActuatorMode_t   mode;   
    ActuatorUnit_t   unit;   
    float            value;  
} ActuatorPrams_struct_t;


typedef enum {
    ACTUATOR_OK = 0,
    ACTUATOR_ERROR_INDEX,
    ACTUATOR_ERROR_MODE,
    ACTUATOR_ERROR_UNIT,
    ACTUATOR_ERROR_VALUE
} errorCode;

void ActuatorHAL_Init(void);
errorCode ActuatorHAL_Set(ActuatorPrams_struct_t actuatorPrams);

#endif
