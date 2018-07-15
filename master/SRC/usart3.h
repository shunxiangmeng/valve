#ifndef __USART3_H
#define __USART3_H	 

#include "misc.h"


void Uart3_Init(u32 bound);
void Uart3_sendStr(char* fmt, ...);
void Uart3_sendData(char* data, uint8_t len);

#endif

