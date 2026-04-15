#include "stm32f303xc.h"
#include "systick.h"

static volatile uint32_t systick_ms = 0;

void systick_init(uint32_t ticks)
{
    SysTick->LOAD = ticks - 1U;
    SysTick->VAL  = 0U;
    SysTick->CTRL = (1U << 2) | (1U << 1) | (1U << 0);
}

uint32_t systick_get_ms(void)
{
    return systick_ms;
}

void SysTick_Handler(void)
{
    systick_ms++;
}
