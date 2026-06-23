#include "../inc/actuator_hal.h"
#include "../../mcu_drivers/inc/drv_gpio.h"
#include "../../mcu_drivers/inc/drv_pwm.h"
#include "../../common/inc/frame_common.h"
#include "../../common/inc/queue_manager.h"

#define FAN_TIM           PWM_TIM3
#define FAN_PORT          PA
#define FAN_PIN           6
#define FAN_CH            PWM_CH1
#define FAN_MAX_RPM       3000.0f

#define HEATER_TIM        PWM_TIM3
#define HEATER_PORT       PA
#define HEATER_PIN        7
#define HEATER_CH         PWM_CH2
#define HEATER_MAX_TEMP_C 100.0f

#define ACT_FREQ          2500

/*
 * Error payload layout (3 bytes):
 *   [0] errorCode  : ACTUATOR_ERROR_xxx
 *   [1] index      : which actuator (ActuatorDevice_t)
 *   [2] mid        : which MID_ACT_ERR_xxx  (redundant but handy)
 */
static errorCode actuator_push_error(errorCode code, uint8_t   mid, ActuatorDevice_t index)
{
    FrameUnion_t err_frame;
    err_frame.data.sof  = FRAME_SOF;
    err_frame.data.type = FRAME_TYPE_RSP;
    err_frame.data.id   = Frame_MakeId(GID_ERROR, mid);
    Frame_SetLength(&err_frame.data, 3);
    err_frame.data.payload[0] = (uint8_t)code;
    err_frame.data.payload[1] = (uint8_t)index;
    err_frame.data.payload[2] = mid;
 
    TX_Queue_Push(&err_frame);
    return code;
}

#define ACT_ERROR(code, mid, index)                          \
    do {                                                     \
        DRV_PWM_SetDuty(FAN_TIM,    FAN_CH,    0);          \
        DRV_PWM_SetDuty(HEATER_TIM, HEATER_CH, 0);          \
        return actuator_push_error((code), (mid), (index));  \
    } while (0)
		
void ActuatorHAL_Init(void)
{
    DRV_PWM_Init(FAN_TIM, FAN_PORT, FAN_PIN, FAN_CH, ACT_FREQ);
    DRV_PWM_Start(FAN_TIM, FAN_CH);
    DRV_PWM_SetDuty(FAN_TIM, FAN_CH, 0);

    DRV_PWM_Init(HEATER_TIM, HEATER_PORT, HEATER_PIN, HEATER_CH, ACT_FREQ);
    DRV_PWM_Start(HEATER_TIM, HEATER_CH);
    DRV_PWM_SetDuty(HEATER_TIM, HEATER_CH, 0);
}

errorCode ActuatorHAL_Set(ActuatorPrams_struct_t actuatorPrams)
{
    if (actuatorPrams.index != ACTUATOR_FAN && actuatorPrams.index != ACTUATOR_HEATER)
    {
        ACT_ERROR(ACTUATOR_ERROR_INDEX, MID_ACT_ERR_INDEX, actuatorPrams.index);
    }

    if (actuatorPrams.mode != ACT_MODE_PWM && actuatorPrams.mode != ACT_MODE_ONOFF)
    {
        ACT_ERROR(ACTUATOR_ERROR_MODE, MID_ACT_ERR_MODE, actuatorPrams.index);
    }

    float percent = 0.0f;

    if (actuatorPrams.index == ACTUATOR_FAN)
    {
        if (actuatorPrams.mode == ACT_MODE_ONOFF)
        {
            if (actuatorPrams.value == 100.0f)
            {
                percent = 100.0f;
            }
            else if (actuatorPrams.value == 0.0f)
            {
                percent = 0.0f;
            }
            else
            {
                ACT_ERROR(ACTUATOR_ERROR_VALUE, MID_ACT_ERR_VALUE, actuatorPrams.index);
            }
        }
        else 
        {
            if (actuatorPrams.unit != UNIT_PERCENT && 
                actuatorPrams.unit != UNIT_RPM && 
                actuatorPrams.unit != UNIT_RPS)
            {
                ACT_ERROR(ACTUATOR_ERROR_UNIT, MID_ACT_ERR_UNIT, actuatorPrams.index);
            }

            switch (actuatorPrams.unit)
            {
                case UNIT_PERCENT:
                    if (actuatorPrams.value < 0.0f || actuatorPrams.value > 100.0f) 
                    {
                        ACT_ERROR(ACTUATOR_ERROR_VALUE, MID_ACT_ERR_VALUE, actuatorPrams.index);
                    }
                    percent = actuatorPrams.value;
                    break;
                case UNIT_RPM:
                    if (actuatorPrams.value < 0.0f || actuatorPrams.value > FAN_MAX_RPM) 
                    {
                        ACT_ERROR(ACTUATOR_ERROR_VALUE, MID_ACT_ERR_VALUE, actuatorPrams.index);
                    }
                    percent = (actuatorPrams.value * 100.0f) / FAN_MAX_RPM;
                    break;
                case UNIT_RPS:
                {
                    float rpm = actuatorPrams.value * 60.0f;
                    if (rpm < 0.0f || rpm > FAN_MAX_RPM) 
                    {
                        ACT_ERROR(ACTUATOR_ERROR_VALUE, MID_ACT_ERR_VALUE, actuatorPrams.index);
                    }
                    percent = (rpm * 100.0f) / FAN_MAX_RPM;
                    break;
                }
                default:
                    ACT_ERROR(ACTUATOR_ERROR_UNIT, MID_ACT_ERR_UNIT, actuatorPrams.index);
            }
        }
    }
    else 
    {
        if (actuatorPrams.mode == ACT_MODE_ONOFF)
        {
            if (actuatorPrams.value == 100.0f)
            {
                percent = 100.0f;
            }
            else if (actuatorPrams.value == 0.0f)
            {
                percent = 0.0f;
            }
            else
            {
                ACT_ERROR(ACTUATOR_ERROR_VALUE, MID_ACT_ERR_VALUE, actuatorPrams.index);
            }
        }
        else 
        {
            if (actuatorPrams.unit != UNIT_PERCENT && 
                actuatorPrams.unit != UNIT_DEG_C && 
                actuatorPrams.unit != UNIT_DEG_F && 
                actuatorPrams.unit != UNIT_DEG_K)
            {
                ACT_ERROR(ACTUATOR_ERROR_UNIT, MID_ACT_ERR_UNIT, actuatorPrams.index);
            }

            float temp_c = 0.0f;
            switch (actuatorPrams.unit)
            {
                case UNIT_PERCENT:
                    if (actuatorPrams.value < 0.0f || actuatorPrams.value > 100.0f) 
                    {
                        ACT_ERROR(ACTUATOR_ERROR_VALUE, MID_ACT_ERR_VALUE, actuatorPrams.index);
                    }
                    percent = actuatorPrams.value;
                    break;
                case UNIT_DEG_C:
                    temp_c = actuatorPrams.value;
                    if (temp_c < 0.0f || temp_c > HEATER_MAX_TEMP_C) 
                    {
                        ACT_ERROR(ACTUATOR_ERROR_VALUE, MID_ACT_ERR_VALUE, actuatorPrams.index);
                    }
                    percent = (temp_c * 100.0f) / HEATER_MAX_TEMP_C;
                    break;
                case UNIT_DEG_F:
                    temp_c = (actuatorPrams.value - 32.0f) * 5.0f / 9.0f;
                    if (temp_c < 0.0f || temp_c > HEATER_MAX_TEMP_C) 
                    {
                        ACT_ERROR(ACTUATOR_ERROR_VALUE, MID_ACT_ERR_VALUE, actuatorPrams.index);
                    }
                    percent = (temp_c * 100.0f) / HEATER_MAX_TEMP_C;
                    break;
                case UNIT_DEG_K:
                    temp_c = actuatorPrams.value - 273.15f;
                    if (temp_c < 0.0f || temp_c > HEATER_MAX_TEMP_C) 
                    {
                        ACT_ERROR(ACTUATOR_ERROR_VALUE, MID_ACT_ERR_VALUE, actuatorPrams.index);
                    }
                    percent = (temp_c * 100.0f) / HEATER_MAX_TEMP_C;
                    break;
                default:
                    ACT_ERROR(ACTUATOR_ERROR_UNIT, MID_ACT_ERR_UNIT, actuatorPrams.index);
            }
        }
    }

    uint16_t duty_cycle = (uint16_t)(percent + 0.5f);

    if (actuatorPrams.index == ACTUATOR_FAN)
    {
        DRV_PWM_SetDuty(FAN_TIM, FAN_CH, duty_cycle);
    }
    else
    {
        DRV_PWM_SetDuty(HEATER_TIM, HEATER_CH, duty_cycle);
    }

    return ACTUATOR_OK;
}
