#include "led.h"

void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure_GPIO_PORTA;
	//使能或APB2外设时钟GPIOA
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

	GPIO_InitStructure_GPIO_PORTA.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_11 | GPIO_Pin_12  ;
	GPIO_InitStructure_GPIO_PORTA.GPIO_Mode  = GPIO_Mode_Out_PP;//推挽输出
	GPIO_InitStructure_GPIO_PORTA.GPIO_Speed = GPIO_Speed_2MHz;//速度2M  
	GPIO_Init(GPIOA,&GPIO_InitStructure_GPIO_PORTA);
	
	GPIO_SetBits(GPIOA,GPIO_Pin_8);		
	GPIO_SetBits(GPIOA,GPIO_Pin_11);	
	GPIO_SetBits(GPIOA,GPIO_Pin_12);	
} 

void LED_Set(uint8_t color)
{
	if(color & RED)
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_12);
	}
	else
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_12);
	}
	
	if(color & GREEN)
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_11);
	}
	else
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_11);
	}
	
	if(color & BLUE)
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_8);
	}
	else
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_8);
	}
}
