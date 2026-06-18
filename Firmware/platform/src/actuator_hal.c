#include "../inc/actuator_hal.h"
#include "../../mcu_drivers/inc/drv_gpio.h"
#include "../../mcu_drivers/inc/drv_pwm.h"

#define FAN_TIM       PWM_TIM3
#define FAN_PORT      PA
#define FAN_PIN       6
#define FAN_CH        PWM_CH1
#define FAN_MAX_RPM   3000.0f

#define HEATER_TIM        PWM_TIM3
#define HEATER_PORT       PA
#define HEATER_PIN        7
#define HEATER_CH         PWM_CH2
#define HEATER_MAX_TEMP_C 100.0f

#define ACT_FREQ      2500

void ActuatorHAL_Init(void)
{
    DRV_PWM_Init(FAN_TIM, FAN_PORT, FAN_PIN, FAN_CH, ACT_FREQ);
    DRV_PWM_Start(FAN_TIM, FAN_CH);
    DRV_PWM_SetDuty(FAN_TIM, FAN_CH, 0);

    DRV_PWM_Init(HEATER_TIM, HEATER_PORT, HEATER_PIN, HEATER_CH, ACT_FREQ);
    DRV_PWM_Start(HEATER_TIM, HEATER_CH);
    DRV_PWM_SetDuty(HEATER_TIM, HEATER_CH, 0);
}

void ActuatorHAL_Set(ActuatorPrams_struct_t actuatorPrams)
{
    float percent = 0.0f;

    if (actuatorPrams.mode == ACT_MODE_ONOFF)
    {
        
        percent = (actuatorPrams.value != 0.0f) ? 100.0f : 0.0f;
    }
    else 
    {
        if (actuatorPrams.index == ACTUATOR_FAN)
        {
            switch (actuatorPrams.unit)
            {
                case UNIT_PERCENT:
                    percent = actuatorPrams.value;
                    break;
                case UNIT_RPM:
                    percent = (actuatorPrams.value * 100.0f) / FAN_MAX_RPM;
                    break;
                case UNIT_RPS:
                {
                    float rpm = actuatorPrams.value * 60.0f;
                    percent = (rpm * 100.0f) / FAN_MAX_RPM;
                    break;
                }
                default:
                    percent = 0.0f;
                    break;
            }
        }
        else 
        {
            float temp_c = 0.0f;
            switch (actuatorPrams.unit)
            {
                case UNIT_PERCENT:
                    percent = actuatorPrams.value;
                    break;
                case UNIT_DEG_C:
                    temp_c = actuatorPrams.value;
                    percent = (temp_c * 100.0f) / HEATER_MAX_TEMP_C;
                    break;
                case UNIT_DEG_F:
                    temp_c = (actuatorPrams.value - 32.0f) * 5.0f / 9.0f;
                    percent = (temp_c * 100.0f) / HEATER_MAX_TEMP_C;
                    break;
                case UNIT_DEG_K:
                    temp_c = actuatorPrams.value - 273.15f;
                    percent = (temp_c * 100.0f) / HEATER_MAX_TEMP_C;
                    break;
                default:
                    percent = 0.0f;
                    break;
            }
        }
    }

    if (actuatorPrams.index == ACTUATOR_FAN)
        DRV_PWM_SetDuty(FAN_TIM, FAN_CH, (uint16_t)percent);
    else
        DRV_PWM_SetDuty(HEATER_TIM, HEATER_CH, (uint16_t)percent);
}
