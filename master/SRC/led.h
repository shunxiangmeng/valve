#ifndef _LED_H
#define _LED_H

#include "misc.h"

#define BLACK 	0x00
#define RED 	0x01
#define GREEN 	0x02
#define BLUE 	0x04
#define WHITE 	0x07
#define YELLOW 	(RED + GREEN)
#define CYAN 	(BLUE + GREEN)
#define VIOLET 	(RED + BLUE)


void LED_Init(void);
void LED_Set(uint8_t color);

#endif

