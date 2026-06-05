#include "actuator_hal.h"
#include "drv_gpio.h"
#include "drv_pwm.h"


#define FAN_TIM      PWM_TIM3
#define FAN_PORT     PA
#define FAN_PIN      6
#define FAN_CH       PWM_CH1

#define LIGHT_TIM    PWM_TIM3
#define LIGHT_PORT   PA
#define LIGHT_PIN    7
#define LIGHT_CH     PWM_CH2

#define ACT_FREQ     2500

void ActuatorHAL_Init(void) {

    DRV_PWM_Init(FAN_TIM, FAN_PORT, FAN_PIN, FAN_CH, ACT_FREQ);
    DRV_PWM_Start(FAN_TIM, FAN_CH);
    DRV_PWM_SetDuty(FAN_TIM, FAN_CH, 0);

    DRV_PWM_Init(LIGHT_TIM, LIGHT_PORT, LIGHT_PIN, LIGHT_CH, ACT_FREQ);
    DRV_PWM_Start(LIGHT_TIM, LIGHT_CH);
    DRV_PWM_SetDuty(LIGHT_TIM, LIGHT_CH, 0);
}

void ActuatorHAL_SetFan(uint8_t duty_percent) {
    if (duty_percent > 100) duty_percent = 100;
    uint16_t duty_val = (uint16_t)duty_percent * 10;


    DRV_PWM_SetDuty(FAN_TIM, FAN_CH, duty_val);
}

void ActuatorHAL_SetLight(uint8_t intensity_percent) {
    if (intensity_percent > 100) intensity_percent = 100;
    uint16_t duty_val = (uint16_t)intensity_percent * 10;

    DRV_PWM_SetDuty(LIGHT_TIM, LIGHT_CH, duty_val);
}
