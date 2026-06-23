#include "../inc/sensor_hal.h"
#include "../../mcu_drivers/inc/drv_timer.h"

float Humidity;
float Temperature;

void DHT22_Start(Ports port, unsigned short pin){
	timerInitForDelay();
	GPIO_Init(port, pin, OUT50, O_GP_PP);
	GPIO_Write(port, pin, 0);
	delay_ms(2);
	GPIO_Write(port, pin, 1);
	delay_us(30);
	GPIO_Init(port, pin, IN, I_PP);
}

int DHT22_Check_Response(Ports port, unsigned short pin){
	int response = 0;
	delay_us(40);
	
	if(GPIO_Read(port, pin) == 0){
		delay_us(80);
		if(GPIO_Read(port, pin) == 1){
			response = 1;
		}
		else{
			response = -1;
		}
	}
	while(GPIO_Read(port, pin) == 1);
	
	return response;
}

uint8_t DHT22_Read_Byte(Ports port, unsigned short pin){
	uint8_t data = 0;
	for(int i = 0; i < 8; i++){
		uint32_t timeout = 10000;
		while(!(GPIO_Read(port, pin))){
			if(--timeout == 0){
				return 0;
			}
		}
		delay_us(40);
		if(GPIO_Read(port, pin) == 1){
			data |= 1 << (7 - i);
			timeout = 10000;
			while(GPIO_Read(port, pin)){
				if(--timeout == 0){
					return 0;;
				}
			}
		}
		else{
			data &= ~(1 << (7 - i));
		}
	}
	return data;
  
}


void DHT22_ReadData(){
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

float readTemperature(){
	return Temperature;
}


