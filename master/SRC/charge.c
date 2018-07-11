#include "charge.h"


void Charge_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure_GPIO_PORTB;
	//使能或APB2外设时钟GPIOA
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure_GPIO_PORTB.GPIO_Pin   = GPIO_Pin_9;
	GPIO_InitStructure_GPIO_PORTB.GPIO_Mode  = GPIO_Mode_Out_PP;//推挽输出
	GPIO_InitStructure_GPIO_PORTB.GPIO_Speed = GPIO_Speed_2MHz;//速度2M  
	GPIO_Init(GPIOB, &GPIO_InitStructure_GPIO_PORTB);
	
	GPIO_SetBits(GPIOB, GPIO_Pin_9);		
}
	
void Charge_On(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_9);	
}

void Charge_Off(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_9);	
}
