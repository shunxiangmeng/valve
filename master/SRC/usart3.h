#ifndef __USART3_H
#define __USART3_H	 

#include "misc.h"


void Uart3_Init(u32 bound);
void Uart3_sendStr(char* fmt, ...);

#endif

