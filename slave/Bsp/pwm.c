/**
  **********************************  STM32F030F4  ***********************************
  * @�ļ���     �� time.c
  * @����       �� liucf
  * @��汾     �� V2.2.0
  * @�ļ��汾   �� V1.0.0
  * @����       �� 2017��04��10��
  * @ժҪ       �� ������ 
  ******************************************************************************/
#include "pwm.h"
#include "Include.h"

  
void TIM3_PWM_Init(uint16_t prescaler)
{ 
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	/* ʹ��GPIOBʱ�� */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	  /* ����LED��Ӧ����PB1*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_1);
	IR_LED_OFF;
	
	
	
	
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʹ�ܶ�ʱ��3ʱ��
    TIM_TimeBaseStructure.TIM_Period        = prescaler;// �����Զ���װ����ֵ 38Khz
    TIM_TimeBaseStructure.TIM_Prescaler     = 0;//����ʱ�ӷ�Ƶϵ��������Ƶ
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//����ʱ�ӷָ�
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;//���ϼ���
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);    //��ʼ����ʱ��3
    
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;// PWM1ģʽ
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_Pulse = 416;   
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;//�����
    TIM_OC4Init(TIM3, &TIM_OCInitStructure);
	
    
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);//ʹ��Ԥװ�ؼĴ���
    
	TIM_ARRPreloadConfig(TIM3, ENABLE);
	TIM_Cmd(TIM3, ENABLE);                          //  ʹ�ܶ�ʱ��3
}


void Timer3_PWM_SetDutyCycle(uint32_t nGUA_Timer3_PWM_DutyCycle)  
{  
  TIM_SetCompare3(TIM3, (48-1)*nGUA_Timer3_PWM_DutyCycle/100);   
}  

