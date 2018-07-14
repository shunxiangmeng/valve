#ifndef __DELAY_H
#define __DELAY_H


#ifndef BIT
#define BIT(x)	(1 << (x))
#endif

void delay_ms(uint16_t nms);
void delay_us(uint32_t nus);
void Delay(uint32_t count);
void Delay_Init(uint8_t SYSCLK);

extern uint16_t fac_ms;//全局变量
extern uint8_t fac_us;//全局变量


#endif

