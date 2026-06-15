#include "../inc/actuator_hal.h"
#include "../../mcu_drivers/inc/drv_gpio.h"
#include "../../mcu_drivers/inc/drv_pwm.h"

#define FAN_TIM      PWM_TIM3
#define FAN_PORT     PA
#define FAN_PIN      6
#define FAN_CH       PWM_CH1

#define HEATER_TIM    PWM_TIM3
#define HEATER_PORT   PA
#define HEATER_PIN    7
#define HEATER_CH     PWM_CH2

#define ACT_FREQ     2500

#define HEATER_MAX_TEMP_C    100.0f 

void ActuatorHAL_Init(void)
{
    DRV_PWM_Init(FAN_TIM, FAN_PORT, FAN_PIN, FAN_CH, ACT_FREQ);
    DRV_PWM_Start(FAN_TIM, FAN_CH);
    DRV_PWM_SetDuty(FAN_TIM, FAN_CH, 0);

    DRV_PWM_Init(HEATER_TIM, HEATER_PORT, HEATER_PIN, HEATER_CH, ACT_FREQ);
    DRV_PWM_Start(HEATER_TIM, HEATER_CH);
    DRV_PWM_SetDuty(HEATER_TIM, HEATER_CH, 0);
}

void ActuatorHAL_SetFan(ActuatorType_t type, float value)
{
    float duty_percent = 0.0f;

    switch (type)
    {
        case UNIT_PERCENT:
            duty_percent = value;
            break;

        case UNIT_RPM:
            duty_percent = (value * 100.0f) / 3000.0f;
            break;

        case UNIT_RPS:
        {
            float rpm = value * 60.0f;
            duty_percent = (rpm * 100.0f) / 3000.0f;
            break;
        }    

        default:
            duty_percent = 0.0f;
            break;
    }
    DRV_PWM_SetDuty(FAN_TIM, FAN_CH, (uint8_t)duty_percent);
}

void ActuatorHAL_SetHeater(ActuatorType_t type, float value)
{
    float intensity_percent = 0.0f;
    float temp_celsius = 0.0f;

    switch (type)
    {
        case UNIT_PERCENT:
            intensity_percent = value;
            break;

        case UNIT_Do_C:
            intensity_percent = (value * 100.0f) / HEATER_MAX_TEMP_C;
            break;

        case UNIT_Do_F: 
            temp_celsius = (value - 32.0f) * 5.0f / 9.0f;
            intensity_percent = (temp_celsius * 100.0f) / HEATER_MAX_TEMP_C;
            break;

        case UNIT_Do_K:
            temp_celsius = value - 273.15f;
            intensity_percent = (temp_celsius * 100.0f) / HEATER_MAX_TEMP_C;
            break;

        default:
            intensity_percent = 0.0f;
            break;
    }
    DRV_PWM_SetDuty(HEATER_TIM, HEATER_CH, (uint8_t)intensity_percent);
}
