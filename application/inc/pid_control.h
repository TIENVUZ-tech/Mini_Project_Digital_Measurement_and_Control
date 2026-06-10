#ifndef PID_CONTROL_H
#define PID_CONTROL_H

#include <stdint.h>

typedef struct {
    float kp;
    float ki;
    float kd;
    
    float integral;      
    float prev_error;    
    
    float out_max;       
    float out_min;       
} PID_Controller_t;

void PID_Init(PID_Controller_t *pid, float kp, float ki, float kd, float min, float max);
float PID_Compute(PID_Controller_t *pid, float setpoint, float feedback);

#endif
