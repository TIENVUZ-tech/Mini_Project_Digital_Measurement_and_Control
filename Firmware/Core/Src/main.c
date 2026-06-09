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
	
	uint8_t data[11] = "Hello world";
	CommHAL_SendBytes(data, 11);
	
	while(1) {
		Protocol_Run(&g_data);
		g_data.temperature += 0.1f;
		//Delay_ms(T2, 2000);
		// Protocol_SendSensorData(&g_data);
	}
}
