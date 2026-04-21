#include "timer.h"
#include "stm32f303xc.h"

static volatile uint32_t g_timer_ms = 0U;

void Timer_InitMs(void)
{
    /* 8 MHz / 1000 = 1 ms tick */
    SysTick_Config(8000000U / 1000U);
}

uint32_t Timer_GetMillis(void)
{
    return g_timer_ms;
}

void SysTick_Handler(void)
{
    g_timer_ms++;
}
