#include "../Inc/main.h"

void SysTick_Handler(void)
{
	ControlApp_SystickUpdate();
}

int main(void)
{
	ControlApp_Init();
 
	while (1)
	{
		ControlApp_Run();
	}
}
