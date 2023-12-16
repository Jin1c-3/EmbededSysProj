#ifndef _time_H
#define _time_H


#include "system.h"

void TIM6_Init(u16 per,u16 psc);
void TIM6_IRQHandler(void);
#endif