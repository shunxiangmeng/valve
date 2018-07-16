#ifndef _usart2_H
#define _usart2_H

#include "misc.h"
#include "stdio.h"

#define DEBUG_TIME 1

#ifdef DEBUG_TIME
	#define PRINT(fmt, ...)  {print(); printf(fmt, ##__VA_ARGS__);}
#elif  DEBUG
	#define PRINT(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#else
	#define PRINT(fmt, ...)  
#endif

void Print_Init(void);
void print(void);

#endif
