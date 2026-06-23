#include "../Inc/main.h"

// Greenhouse data
static GreenhouseData_t g_data = {
	.temperature        = 28.5f,
	.temp_setpoint      = 25.0f,
	.fan_state          = 0,
	.heater_state       = 0,
	.fan_pwm            = 0,
	.heater_pwm         = 0,
	.mode               = MODE_AUTO,
	.report_mode        = REPORT_MODE_ONE_SHOT,
	.report_once_pending = 1
};

// Soft timmer
static SoftTimer_t s_report_timer;

// Flags set by timer callbacks, cleared by main loop
static volatile uint8_t s_report_flag = 0;

// Timer callbacks
static void on_report_timer(void) {
	s_report_flag = 1;
}

void SysTick_Handler(void)
{
	SoftTimer_Update(&s_report_timer);
	Protocol_SystickUpdate();   // update rx + tx timeout 
}

int main(void)
{
	SystickInit();
	Protocol_Init();
	ActuatorHAL_Init();
 
	SoftTimer_Start(&s_report_timer, 2000, on_report_timer);
 
	while (1)
	{
		SoftTimer_Dispatch(&s_report_timer);
 
		Protocol_Run(&g_data);
 
		if (s_report_flag)
		{
			s_report_flag = 0;
 
			if (g_data.report_mode == REPORT_MODE_STREAM)
			{
				Protocol_SendSensorData(&g_data);
			}
			else if (g_data.report_once_pending)
			{
				g_data.report_once_pending = 0;
				Protocol_SendSensorData(&g_data);
			}
		}
	}
}
