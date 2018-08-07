
#include "stm32f0xx.h"
#include "delay.h"

uint32_t fac_ms;//ȫ�ֱ���
uint32_t fac_us;//ȫ�ֱ���

/****************************************************
�������ܣ�ms����ʱ
���������nms : ����
�����������
��    ע�����ô˺���ǰ����Ҫ��ʼ��Delay_Init()����
*****************************************************/							    
void delay_ms(uint32_t nms)
{
   	SysTick->LOAD = (uint32_t)fac_ms*nms-1;//����ʱ��ֵ
	  SysTick->VAL = 1;//���д��ֵ��������ؼĴ�����ֵ
	  SysTick->CTRL |= BIT(0);//SysTickʹ��
 	  while(!(SysTick->CTRL&(1<<16)));//�ж��Ƿ����0
	  SysTick->CTRL &=~BIT(0);//�ر�SysTick
}

/****************************************************
�������ܣ���ʱ��ʼ��
���������SYSCLK : ϵͳʱ��(48)MHZ
�����������
��    ע����
*****************************************************/
void Delay_Init(uint32_t SYSCLK)
{
   SysTick->CTRL &=~BIT(2);//ѡ���ⲿʱ��
	 SysTick->CTRL &=~BIT(1);//�رն�ʱ������0����ж�����
	 fac_us = SYSCLK/8;//�����SysTick����ֵ   6
	 fac_ms = (uint32_t)fac_us*1000;	 
}

/****************************************************
�������ܣ�us����ʱ
���������nus : ΢��
�����������
��    ע�����ô˺���ǰ����Ҫ��ʼ��Delay_Init()����
*****************************************************/		    								   
void delay_us(uint32_t nus)
{		
	  SysTick->LOAD = (uint32_t)fac_us*nus-1;//����ʱ��ֵ
	  SysTick->VAL = 1;//���д��ֵ��������ؼĴ�����ֵ
	  SysTick->CTRL |= BIT(0);//SysTickʹ��
	  while(!(SysTick->CTRL&(1<<16)));//�ж��Ƿ����0
	  SysTick->CTRL &=~BIT(0);//�ر�SysTick
}


