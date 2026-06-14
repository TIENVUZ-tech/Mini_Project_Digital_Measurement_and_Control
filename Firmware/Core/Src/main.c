#include "../Inc/main.h"

static GreenhouseData_t g_data = {
	.temperature = 28.5f,
	.temp_setpoint = 25.0f,
	.fan_state = 0,
	.heater_state = 0,
	.fan_pwm = 0,
	.mode = MODE_AUTO
};

int main() {
	CommHAL_Init();
	Protocol_Init();
	TIM2_Init();
	
	uint8_t data[11] = "Hello world";
	CommHAL_SendBytes(data, 11);
	
	uint32_t last_report_ms = 0;
	
	while(1) {
		Protocol_Run(&g_data);
		
		uint32_t now = DRV_TIM_GetMs();
		if (now - last_report_ms >= 2000) {
			last_report_ms = now;
			Protocol_SendSensorData(&g_data);
		}
	}
}
