#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

/* Initialise the SysTick timer */
void systick_init(uint32_t ticks);

/* Get the current time in milliseconds */
uint32_t systick_get_ms(void);

#endif // SYSTICK_H
