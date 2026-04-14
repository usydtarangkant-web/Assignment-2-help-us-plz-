#include "systick.h"

#define SYSTICK_BASE 0xE000E010UL

typedef struct
{
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_Type;

#define SYSTICK ((SysTick_Type *)SYSTICK_BASE)

// CTRL bits
#define SYSTICK_CTRL_ENABLE     (1UL << 0)
#define SYSTICK_CTRL_TICKINT    (1UL << 1)
#define SYSTICK_CTRL_CLKSOURCE  (1UL << 2)

static volatile uint32_t systick_ms = 0;

void systick_init(uint32_t ticks)
{
    SYSTICK->LOAD = ticks - 1U;
    SYSTICK->VAL  = 0U;
    SYSTICK->CTRL = SYSTICK_CTRL_CLKSOURCE |
                    SYSTICK_CTRL_TICKINT   |
                    SYSTICK_CTRL_ENABLE;
}

uint32_t systick_get_ms(void)
{
    return systick_ms;
}

void SysTick_Handler(void)
{
    systick_ms++;
}
