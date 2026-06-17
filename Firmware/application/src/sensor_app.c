#include "stm32f10x.h"
#include "soft_timer.h"
#include "sensor_hal.h"
#include "drv_timer.h"

#define DHT22_Port PA
#define DHT22_Pin 0

SoftTimer_t DHT22_timer;
SoftTimer_t led_timer;
uint8_t DHT22_Data[5];

float Humidity;
float Temperature;

void DHT22_task(){
	DHT22_Start(DHT22_Port, DHT22_Pin);
	if(DHT22_Check_Response(DHT22_Port, DHT22_Pin) == 1){
		DHT22_Data[0] = DHT22_Read_Byte(DHT22_Port, DHT22_Pin); //Do am nguyen
		DHT22_Data[1] = DHT22_Read_Byte(DHT22_Port, DHT22_Pin); //Do am thap phan
		DHT22_Data[2] = DHT22_Read_Byte(DHT22_Port, DHT22_Pin); //Nhiet do nguyen
		DHT22_Data[3] = DHT22_Read_Byte(DHT22_Port, DHT22_Pin); //Nhiet do thap phan
		DHT22_Data[4] = DHT22_Read_Byte(DHT22_Port, DHT22_Pin); //Check sum
	}

	uint8_t sum = DHT22_Data[0] + DHT22_Data[1] +DHT22_Data[2] + DHT22_Data[3];
	if(DHT22_Data[4] == sum){
		uint16_t rawHumidity = (DHT22_Data[0] << 8) | DHT22_Data[1];
		uint16_t rawTemperature = (DHT22_Data[2] << 8) | DHT22_Data[3];
		Humidity = (float) rawHumidity/10.0;
		Temperature = (float) rawTemperature/10.0;
	}

}

/*void toggle_task(){
	GPIO_Toggle(PC,13);
}*/

void SysTick_Handler(){
	SoftTimer_Update(&DHT22_timer);
//	SoftTimer_Update(&led_timer);
}

/*int main(){
	timerInitForDelay();
	SystickInit();
	GPIO_Init(PC, 13, OUT50, O_GP_PP);
	SoftTimer_Start(&DHT22_timer, 2500, DHT22_task);
	SoftTimer_Start(&led_timer, 2500, toggle_task);

	while(1){
		SoftTimer_Dispatch(&DHT22_timer);
		SoftTimer_Dispatch(&led_timer);
	}

}*/
