//=============================================================================
//�ļ����ƣ�LED.h
//���ܸ�Ҫ��LED����ͷ�ļ�
//��Ȩ���У�Դ�ع�����www.vcc-gnd.com
//�Ա����꣺http://vcc-gnd.taobao.com
//����ʱ�䣺2013-12-31
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
