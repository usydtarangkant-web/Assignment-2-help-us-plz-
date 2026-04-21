#include "led.h"

/* Discovery LEDs:
   PE8  = LD4
   PE9  = LD3
   PE10 = LD5
   PE11 = LD7
   PE12 = LD9
   PE13 = LD10
   PE14 = LD8
   PE15 = LD6
*/

static const uint16_t ledPins[8] = {8, 9, 10, 11, 12, 13, 14, 15};

void LED_Init(void)
{
    uint8_t i;

    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;

    for (i = 0; i < 8; i++)
    {
        GPIOE->MODER &= ~(3U << (ledPins[i] * 2U));
        GPIOE->MODER |=  (1U << (ledPins[i] * 2U));
        GPIOE->OTYPER &= ~(1U << ledPins[i]);
    }

    LED_AllOff();
}

void LED_AllOff(void)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        GPIOE->ODR &= ~(1U << ledPins[i]);
    }
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

/* Show one of 8 heading sectors on the LED ring */
void LED_ShowHeading8(uint16_t heading_deg)
{
    uint8_t sector;

    /* 360 / 8 = 45 degrees per LED */
    sector = (uint8_t)(((heading_deg + 22U) % 360U) / 45U);

    LED_AllOff();
    LED_On(ledPins[sector]);
}
