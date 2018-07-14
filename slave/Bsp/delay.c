
#include "stm32f0xx.h"
#include "delay.h"

uint16_t fac_ms;//全局变量
uint8_t fac_us;//全局变量

/****************************************************
函数功能：ms级延时
输入参数：nms : 毫秒
输出参数：无
备    注：调用此函数前，需要初始化Delay_Init()函数
*****************************************************/							    
void delay_ms(uint16_t nms)
{
   	SysTick->LOAD = (uint32_t)fac_ms*nms-1;//加载时间值
	  SysTick->VAL = 1;//随便写个值，清除加载寄存器的值
	  SysTick->CTRL |= BIT(0);//SysTick使能
 	  while(!(SysTick->CTRL&(1<<16)));//判断是否减到0
	  SysTick->CTRL &=~BIT(0);//关闭SysTick
}

/****************************************************
函数功能：延时初始化
输入参数：SYSCLK : 系统时钟(72)MHZ
输出参数：无
备    注：无
*****************************************************/
void Delay_Init(uint8_t SYSCLK)
{
   SysTick->CTRL &=~BIT(2);//选择外部时钟
	 SysTick->CTRL &=~BIT(1);//关闭定时器减到0后的中断请求
	 fac_us = SYSCLK/8;//计算好SysTick加载值
	 fac_ms = (uint16_t)fac_us*1000;	 
}

/****************************************************
函数功能：us级延时
输入参数：nus : 微秒
输出参数：无
备    注：调用此函数前，需要初始化Delay_Init()函数
*****************************************************/		    								   
void delay_us(uint32_t nus)
{		
	  SysTick->LOAD = (uint32_t)fac_us*nus-1;//加载时间值
	  SysTick->VAL = 1;//随便写个值，清除加载寄存器的值
	  SysTick->CTRL |= BIT(0);//SysTick使能
	  while(!(SysTick->CTRL&(1<<16)));//判断是否减到0
	  SysTick->CTRL &=~BIT(0);//关闭SysTick
}


