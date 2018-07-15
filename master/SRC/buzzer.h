#ifndef _buzzer_H
#define _buzzer_H

#include "misc.h"

#define ON  1
#define OFF 0

void BUZZER_Init(void);
void BUZZER_Set(uint8_t onoff);
void BUZZER_start(uint32_t onTime, uint32_t offTime, uint32_t loop);
void BUZZER_stop(void);

#endif

