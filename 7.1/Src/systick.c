#include "stm32f303xc.h"
#include "systick.h"

/* Millisecond counter */
static volatile uint32_t systick_ms = 0;

/* Initialise the SysTick timer */
void systick_init(uint32_t ticks)
{
    SysTick->LOAD = ticks - 1U;
    SysTick->VAL  = 0U;
    SysTick->CTRL = (1U << 2) | (1U << 1) | (1U << 0);
}

/* Get the current time in milliseconds */
uint32_t systick_get_ms(void)
{
    return systick_ms;
}

/* Increase the millisecond counter every SysTick interrupt */
void SysTick_Handler(void)
{
    systick_ms++;
}
