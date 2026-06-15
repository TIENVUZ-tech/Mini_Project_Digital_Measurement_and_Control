#include "../inc/actuator_hal.h"
#include "../../mcu_drivers/inc/drv_gpio.h"
#include "../../mcu_drivers/inc/drv_pwm.h"

#define FAN_TIM      PWM_TIM3
#define FAN_PORT     PA
#define FAN_PIN      6
#define FAN_CH       PWM_CH1

#define LIGHT_TIM    PWM_TIM3
#define LIGHT_PORT   PA
#define LIGHT_PIN    7
#define LIGHT_CH     PWM_CH2

#define ACT_FREQ     2500

void ActuatorHAL_Init(void)
{
    DRV_PWM_Init(FAN_TIM, FAN_PORT, FAN_PIN, FAN_CH, ACT_FREQ);
    DRV_PWM_Start(FAN_TIM, FAN_CH);
    DRV_PWM_SetDuty(FAN_TIM, FAN_CH, 0);

    DRV_PWM_Init(LIGHT_TIM, LIGHT_PORT, LIGHT_PIN, LIGHT_CH, ACT_FREQ);
    DRV_PWM_Start(LIGHT_TIM, LIGHT_CH);
    DRV_PWM_SetDuty(LIGHT_TIM, LIGHT_CH, 0);
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

        case UNIT_RAW:
            duty_percent = value;
            break;

        default:
            duty_percent = 0.0f;
            break;
    }
    if (duty_percent > 100.0f) duty_percent = 100.0f;
    if (duty_percent < 0.0f)   duty_percent = 0.0f;
    DRV_PWM_SetDuty(FAN_TIM, FAN_CH, (uint8_t)duty_percent);
}

void ActuatorHAL_SetLight(ActuatorType_t type, float value)
{
    float intensity_percent = 0.0f;

    switch (type)
    {
        case UNIT_PERCENT:
            intensity_percent = value;
            break;

        case UNIT_LUX:
            intensity_percent = (value * 100.0f) / 500.0f;
            break;

        case UNIT_RAW:
            intensity_percent = value;
            break;

        default:
            intensity_percent = 0.0f;
            break;
    }
    DRV_PWM_SetDuty(LIGHT_TIM, LIGHT_CH, (uint8_t)intensity_percent);
}
