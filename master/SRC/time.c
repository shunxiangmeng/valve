#include "time.h"

__IO u32 gTimingCount = 0;

TIME_FUNC_INFO gTimeFunc[MAX_FUNCTION_NUM] = {0};

void rtc_Init(void)
{

    BKP_DeInit();
	RCC_LSEConfig(RCC_LSE_ON);
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {}
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    RCC_RTCCLKCmd(ENABLE);
    RTC_WaitForSynchro();           
    RTC_WaitForLastTask();
    RTC_SetPrescaler(32767);
    RTC_WaitForLastTask();
    RTC_ITConfig(RTC_IT_SEC, ENABLE);     
    RTC_WaitForLastTask();
}

void TIM4_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); 
	
	TIM_DeInit(TIM4);
	TIM_TimeBaseStructure.TIM_Period = 1000-1;               //自动重装载寄存器的值
	TIM_TimeBaseStructure.TIM_Prescaler = (72-1);            //时钟预分频数   1000K
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  //采样分频
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);                    //清除溢出中断标志
	TIM_ITConfig(TIM4, TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM4, DISABLE);                                  //开启时钟
	
	// Enable the TIME4 Interrupt 
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
	
	TIM_Cmd(TIM4, ENABLE);                                  //开启时钟
}

//1ms一次中断
void TIM4_IRQHandler(void)
{
	uint32_t i;
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) // check interrupt source
	{
		gTimingCount++;
		
		for (i = 0; i < MAX_FUNCTION_NUM; i++)
		{
			if (gTimeFunc[i].count != 0)
			{
				if (gTimingCount % gTimeFunc[i].time == 0)
				{
					gTimeFunc[i].pfn(gTimeFunc[i].arg);
					gTimeFunc[i].count--;
				}
			}
		}
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);   // clear UIF flag
	}
}


void Time_Init(void)
{
	TIM4_Init();
}
