#include "pwm.h"
#include "stm32f303xc.h"

void PWM_InitServoPA5(void)
{
    RCC->AHBENR  |= RCC_AHBENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* PA5 = AF1 = TIM2_CH1 */
    GPIOA->MODER &= ~(3U << (5U * 2U));
    GPIOA->MODER |=  (2U << (5U * 2U));

    GPIOA->AFR[0] &= ~(0xFU << (5U * 4U));
    GPIOA->AFR[0] |=  (1U << (5U * 4U));

    /* Timer clock assumed 8 MHz
       Prescaler 7 -> 1 MHz timer tick
       ARR 19999 -> 20 ms period (50 Hz) */
    TIM2->PSC = 7U;
    TIM2->ARR = 19999U;

    /* PWM mode 1 on channel 1 */
    TIM2->CCMR1 &= ~(7U << 4U);
    TIM2->CCMR1 |=  (6U << 4U);
    TIM2->CCMR1 |=  TIM_CCMR1_OC1PE;

    TIM2->CCER |= TIM_CCER_CC1E;

    /* Start centred at 1.5 ms */
    TIM2->CCR1 = 1500U;

    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->EGR |= TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;
}

void PWM_SetPulseUs(uint16_t pulse_us)
{
    if (pulse_us < 1000U)
    {
        pulse_us = 1000U;
    }

    if (pulse_us > 2000U)
    {
        pulse_us = 2000U;
    }

    TIM2->CCR1 = pulse_us;
}

/* Simple scaling from heading to servo pulse
   0 deg -> 1000 us
   359 deg -> about 2000 us */
void PWM_SetFromHeading(uint16_t heading_deg)
{
    uint32_t pulse_us;

    if (heading_deg > 359U)
    {
        heading_deg = 359U;
    }

    pulse_us = 1000U + ((uint32_t)heading_deg * 1000U) / 359U;
    PWM_SetPulseUs((uint16_t)pulse_us);
}
