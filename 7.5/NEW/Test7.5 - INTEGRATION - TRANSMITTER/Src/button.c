#include "button.h"
#include "stm32f303xc.h"

extern volatile uint32_t tick_ms;

/* Saved callback function */
static button_callback_t s_button_callback = 0;

/* Used for simple debounce */
static volatile uint32_t s_last_press_ms = 0U;

void Button_InitInterrupt(button_callback_t callback)
{
    s_button_callback = callback;

    RCC->AHBENR  |= RCC_AHBENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* PA0 as input */
    GPIOA->MODER &= ~(3U << 0U);
    GPIOA->PUPDR &= ~(3U << 0U);

    /* Connect EXTI0 to PA0 */
    SYSCFG->EXTICR[0] &= ~(0xFU << 0U);

    /* Enable rising edge interrupt */
    EXTI->IMR  |= (1U << 0U);
    EXTI->RTSR |= (1U << 0U);
    EXTI->FTSR &= ~(1U << 0U);

    EXTI->PR = (1U << 0U);

    NVIC_EnableIRQ(EXTI0_IRQn);
}

void EXTI0_IRQHandler(void)
{
    if (EXTI->PR & (1U << 0U))
    {
        EXTI->PR = (1U << 0U);

        /* Simple debounce: ignore very fast repeats */
        if ((tick_ms - s_last_press_ms) >= 200U)
        {
            s_last_press_ms = tick_ms;

            if (s_button_callback != 0)
            {
                s_button_callback();
            }
        }
    }
}
