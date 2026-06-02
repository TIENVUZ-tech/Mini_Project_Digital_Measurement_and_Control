#ifndef GREENHOUSE_TYPES_H
#define GREENHOUSE_TYPES_H

#include <stdint.h>

typedef enum {
    STATE_IDLE = 0,   
    STATE_AUTO,       
    STATE_MANUAL,     
    STATE_ERROR       
} SystemState_t;

typedef enum {
    ACT_OFF = 0,
    ACT_ON
} ActuatorState_t;

typedef struct {
    float temperature;      
    float air_humidity;     
    float soil_moisture;    

    float temp_setpoint;    
    float hum_setpoint;     

    uint8_t fan_state;      
    uint16_t fan_pwm;       
    uint8_t heater_state;   
    uint8_t mist_state;     

    SystemState_t state;    
} GreenhouseData_t;

#endif // GREENHOUSE_TYPES_H
