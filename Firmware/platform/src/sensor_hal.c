#include "sensor_hal.h"
#include "drv_timer.h"

void DHT22_Start(Ports port, unsigned short pin){
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

