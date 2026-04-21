#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/* Simple 1 ms system timer using SysTick */
void Timer_InitMs(void);
uint32_t Timer_GetMillis(void);

#endif
