/**
  **********************************  STM32F030F4  ***********************************
  * @文件名     ： time.c
  * @作者       ： liucf
  * @库版本     ： V2.2.0
  * @文件版本   ： V1.0.0
  * @日期       ： 2017年04月10日
  * @摘要       ： 主函数 
  ******************************************************************************/
#include "pwm.h"
#include "Include.h"

  
void TIM3_PWM_Init(uint16_t prescaler)
{ 
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	/* 使能GPIOB时钟 */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	  /* 配置LED相应引脚PB1*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_1);
	IR_LED_OFF;
	
	
	
	
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //使能定时器3时钟
    TIM_TimeBaseStructure.TIM_Period        = prescaler;// 设置自动重装周期值 38Khz
    TIM_TimeBaseStructure.TIM_Prescaler     = 0;//设置时钟分频系数：不分频
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//设置时钟分割
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);    //初始化定时器3
    
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;// PWM1模式
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//比较输出使能
	TIM_OCInitStructure.TIM_Pulse = 416;   
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;//输出高
    TIM_OC4Init(TIM3, &TIM_OCInitStructure);
	
    
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);//使能预装载寄存器
    
	TIM_ARRPreloadConfig(TIM3, ENABLE);
	TIM_Cmd(TIM3, ENABLE);                          //  使能定时器3
}


void Timer3_PWM_SetDutyCycle(uint32_t nGUA_Timer3_PWM_DutyCycle)  
{  
  TIM_SetCompare3(TIM3, (48-1)*nGUA_Timer3_PWM_DutyCycle/100);   
}  

