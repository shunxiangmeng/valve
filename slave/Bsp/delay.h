#ifndef __DELAY_H
#define __DELAY_H


#ifndef BIT
#define BIT(x)	(1 << (x))
#endif

void delay_ms(uint32_t nms);
void delay_us(uint32_t nus);
void Delay(uint32_t count);
void Delay_Init(uint32_t SYSCLK);

extern uint32_t fac_ms;//全局变量
extern uint32_t fac_us;//全局变量


#endif

