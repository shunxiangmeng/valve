#include "buzzer.h"

void BUZZER_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure_GPIO_PORTC;
	//使能或APB2外设时钟GPIOC
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	GPIO_InitStructure_GPIO_PORTC.GPIO_Pin   = GPIO_Pin_13  ;
	GPIO_InitStructure_GPIO_PORTC.GPIO_Mode  = GPIO_Mode_Out_PP;//推挽输出
	GPIO_InitStructure_GPIO_PORTC.GPIO_Speed = GPIO_Speed_2MHz;//速度2M  
	GPIO_Init(GPIOC,&GPIO_InitStructure_GPIO_PORTC);
} 

void BUZZER_Set(uint8_t onoff)
{
	if(onoff)
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
	}
	else
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	}
}
