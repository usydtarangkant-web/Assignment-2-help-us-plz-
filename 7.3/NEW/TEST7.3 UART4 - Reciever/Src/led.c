#include "led.h"

/* Discovery LEDs
   PE8  = LD4 blue
   PE9  = LD3 red
   PE10 = LD5 orange
   PE15 = LD6 green */

void LED_Init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;

    GPIOE->MODER &= ~(3U << (8 * 2));
    GPIOE->MODER &= ~(3U << (9 * 2));
    GPIOE->MODER &= ~(3U << (10 * 2));
    GPIOE->MODER &= ~(3U << (15 * 2));

    GPIOE->MODER |=  (1U << (8 * 2));
    GPIOE->MODER |=  (1U << (9 * 2));
    GPIOE->MODER |=  (1U << (10 * 2));
    GPIOE->MODER |=  (1U << (15 * 2));

    GPIOE->ODR &= ~(1U << 8);
    GPIOE->ODR &= ~(1U << 9);
    GPIOE->ODR &= ~(1U << 10);
    GPIOE->ODR &= ~(1U << 15);
}

void LED_On(uint16_t pin)
{
    GPIOE->ODR |= (1U << pin);
}

void LED_Off(uint16_t pin)
{
    GPIOE->ODR &= ~(1U << pin);
}

void LED_Toggle(uint16_t pin)
{
    GPIOE->ODR ^= (1U << pin);
}

void LED_Delay(volatile uint32_t count)
{
    while (count > 0)
    {
        count--;
    }
}
