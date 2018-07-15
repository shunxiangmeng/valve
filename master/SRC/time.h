#ifndef _TIME_HH
#define _TIME_HH

#include "misc.h"

typedef void (*pfn_TIME)(uint32_t); 

#define MAX_FUNCTION_NUM	20

typedef struct
{
	uint32_t time;
	uint32_t count;
	uint32_t arg;
	pfn_TIME pfn;
}TIME_FUNC_INFO;

extern TIME_FUNC_INFO gTimeFunc[MAX_FUNCTION_NUM];

void Time_Init(void);

#endif

