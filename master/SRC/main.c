#include <stdio.h>
#include "SysTick.h"
#include "bluetooth.h"
#include "usart2.h"
#include "led.h"
#include "buzzer.h"
#include "charge.h"

int main(void)
{

	LED_Init();
	BUZZER_Init();
	Charge_Init();
	Usart2_Init();
	
	BUZZER_Set(ON);
	BUZZER_Set(OFF);
	
	Charge_On();
	LED_Set(BLUE);
	
	Charge_Off();
	LED_Set(0);
	
	
	printf("hello\r\n");
	
	BLE_Init();
	

	while(1)
	{
		Delay_ms(1000);
		Delay_ms(1000);
	}
	
	return 0;
}
