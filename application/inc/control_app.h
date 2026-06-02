#ifndef CONTROL_APP_H
#define CONTROL_APP_H

#include "greenhouse_types.h"

// C?u trúc b? ði?u khi?n PID cho STM32F103
typedef struct {
    float kp;
    float ki;
    float kd;
    float integral;
    float prev_error;
    float max_output;
    float min_output;
    float max_integral;
} PID_Controller_t;

void ControlApp_Init(void);
void ControlApp_Update(GreenhouseData_t *data);
void ControlApp_SetMode(SystemState_t state);
void ControlApp_SetTempSetpoint(float temp);
void ControlApp_SetHumSetpoint(float hum);
void ControlApp_SetPIDTune(float p, float i, float d);

#endif // CONTROL_APP_H
