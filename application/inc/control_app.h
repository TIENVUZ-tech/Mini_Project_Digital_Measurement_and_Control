#ifndef CONTROL_APP_H
#define CONTROL_APP_H

#include <stdint.h>


typedef enum {
    STATE_IDLE = 0,
    STATE_AUTO,
    STATE_MANUAL,
    STATE_ERROR
} GreenhouseState_t;


typedef struct {
    GreenhouseState_t state;
    float current_temperature;
    float current_humidity;
    uint16_t fan_pwm;
    uint8_t heater_state;
} GreenhouseData_t;


void ControlApp_Init(void);
void ControlApp_Run(void) ;

void ControlApp_SetPID(float kp, float ki, float kd);


GreenhouseData_t* ControlApp_GetData(void);

#endif
