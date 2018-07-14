#include <stdio.h>
#include "SysTick.h"
#include "bluetooth.h"
#include "usart2.h"
#include "led.h"
#include "buzzer.h"
#include "charge.h"
#include "ir.h"

uint8_t password[7] = {"654321"};
uint8_t count = 0;

int main(void)
{

	LED_Init();
	BUZZER_Init();
	Charge_Init();
	Usart2_Init();
	IR_Init();
	
	BUZZER_Set(ON);
	BUZZER_Set(OFF);
	
	Charge_On();
	LED_Set(BLUE);
	
	Charge_Off();
//	LED_Set(0);
	
	
	printf("hello\r\n");
	
	BLE_Init();
	

	while(1)
	{
		Delay_ms(1);
		if(gIR.rxFlag)
		{
			switch(gIR.rxCmd)
			{
				case 0x0A:
					count++;
					IR_SendCMD(0x02, gIR.randomCode, password, 6);
					break;
				case 0x02:
					IR_SendAck(gIR.randomCode, 0);
					break;
				default:break;
			}
			
		}
	}
	
	return 0;
}
