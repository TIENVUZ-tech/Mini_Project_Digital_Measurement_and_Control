#include "../inc/actuator_hal.h"
#include "../../mcu_drivers/inc/drv_gpio.h"
#include "../../mcu_drivers/inc/drv_pwm.h"

void Delay(volatile uint32_t t)
{
    while(t--);
}

int main(void)
{
    ActuatorHAL_Init();

    while(1)
    {
        
        ActuatorHAL_SetFan(UNIT_RPS, 0);
        ActuatorHAL_SetLight(UNIT_LUX, 0);
        Delay(3000000);

        
        ActuatorHAL_SetFan(UNIT_RPS, 12.5f);
        ActuatorHAL_SetLight(UNIT_LUX, 125);
        Delay(3000000);

       
        ActuatorHAL_SetFan(UNIT_RPS, 25);
        ActuatorHAL_SetLight(UNIT_LUX, 250);
        Delay(3000000);

        
        ActuatorHAL_SetFan(UNIT_RPS, 37.5f);
        ActuatorHAL_SetLight(UNIT_LUX, 375);
        Delay(3000000);

        
        ActuatorHAL_SetFan(UNIT_RPS, 50);
        ActuatorHAL_SetLight(UNIT_LUX, 500);
        Delay(3000000);
    }
}
