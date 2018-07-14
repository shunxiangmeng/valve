//=============================================================================
//文件名称：LED.h
//功能概要：LED驱动头文件
//版权所有：源地工作室www.vcc-gnd.com
//淘宝网店：http://vcc-gnd.taobao.com
//更新时间：2013-12-31
//=============================================================================
#ifndef __LED_H
#define __LED_H

#include "stm32f0xx.h"


/* LED */
#define LED_ON                    GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define LED_OFF                   GPIO_ResetBits(GPIOA, GPIO_Pin_4)


#define MAGNETIC_ON               GPIO_SetBits(GPIOA, GPIO_Pin_10)
#define MAGNETIC_OFF              GPIO_ResetBits(GPIOA, GPIO_Pin_10)



void LED_Init(void);

#endif
