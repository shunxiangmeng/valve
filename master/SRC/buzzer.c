#include "buzzer.h"

struct
{
	uint32_t loopCount;
	uint32_t onTime;
	uint32_t offTime;
	uint32_t status;
	uint32_t time;  //ms
}gBUZZER;

void BUZZER_TimerInit(void);

void BUZZER_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure_GPIO_PORTC;
	//使能或APB2外设时钟GPIOC
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	GPIO_InitStructure_GPIO_PORTC.GPIO_Pin   = GPIO_Pin_13  ;
	GPIO_InitStructure_GPIO_PORTC.GPIO_Mode  = GPIO_Mode_Out_PP;//推挽输出
	GPIO_InitStructure_GPIO_PORTC.GPIO_Speed = GPIO_Speed_2MHz;//速度2M  
	GPIO_Init(GPIOC,&GPIO_InitStructure_GPIO_PORTC);
	
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	
	BUZZER_TimerInit();
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

void BUZZER_TimerInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); 
	
	TIM_DeInit(TIM2);
	TIM_TimeBaseStructure.TIM_Period = 1000-1;               //自动重装载寄存器的值
	TIM_TimeBaseStructure.TIM_Prescaler = (72-1);           //时钟预分频数   1000K
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  //采样分频
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);                    //清除溢出中断标志
	TIM_ITConfig(TIM2, TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM2, DISABLE);                                  //开启时钟
	
	// Enable the TIME2 Interrupt 
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
}

//1ms一次中断
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) // check interrupt source
	{
		gBUZZER.time++;
		
		if (gBUZZER.time == gBUZZER.onTime)
		{
			BUZZER_Set(0);
		}
		else if (gBUZZER.time == gBUZZER.offTime + gBUZZER.onTime)
		{
			BUZZER_Set(1);
			gBUZZER.time = 0;
			gBUZZER.loopCount--;
			if (gBUZZER.loopCount == 0)
			{
				BUZZER_Set(0);
				TIM_Cmd(TIM2, DISABLE);
			}
		}
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);   // clear UIF flag
	}
}

void BUZZER_start(uint32_t onTime, uint32_t offTime, uint32_t loop)
{
	gBUZZER.loopCount = loop;
	gBUZZER.onTime = onTime;
	gBUZZER.offTime = offTime;
	gBUZZER.time = 0;
	BUZZER_Set(1);
	TIM_Cmd(TIM2, ENABLE);
}

void BUZZER_stop(void)
{
	gBUZZER.loopCount = 0;
	BUZZER_Set(0);
	TIM_Cmd(TIM2, DISABLE);
}
