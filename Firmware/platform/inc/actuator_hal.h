##ifndef __ACTUATOR_HAL_H
#define __ACTUATOR_HAL_H

#include <stdint.h>


typedef enum {
    UNIT_PERCENT,
    UNIT_RPM,
    UNIT_RPS,
    UNIT_LUX,
    UNIT_RAW
} ActuatorType_t;

void ActuatorHAL_Init(void);

void ActuatorHAL_SetFan(ActuatorType_t type, float value);
void ActuatorHAL_SetLight(ActuatorType_t type, float value);

#endif
