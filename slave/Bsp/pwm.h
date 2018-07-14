#ifndef __PWM_H
#define	__PWM_H

#include "stm32f0xx.h"

#define TIMER1_PWM_STATUS_ON 1
#define TIMER1_PWM_STATUS_OFF 1
#define IR_LED_ON                    GPIO_SetBits(GPIOB, GPIO_Pin_1)
#define IR_LED_OFF                   GPIO_ResetBits(GPIOB, GPIO_Pin_1)

//void BEEP_Init(void);
void TIM3_PWM_Init(uint16_t prescaler);
void Timer3_PWM_SetDutyCycle(uint32_t nGUA_Timer3_PWM_DutyCycle);  
#endif
