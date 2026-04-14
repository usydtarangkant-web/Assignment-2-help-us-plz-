#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

void systick_init(uint32_t ticks);
uint32_t systick_get_ms(void);

#endif // SYSTICK_H
