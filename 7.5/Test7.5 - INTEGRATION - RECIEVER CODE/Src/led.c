#include "led.h"

static const uint16_t s_led_pins[LED_COUNT] =
{
    8U, 9U, 10U, 11U
};

void LED_Init(void)
{
    uint32_t i;
    uint32_t pin;

    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;

    for (i = 0U; i < LED_COUNT; i++)
    {
        pin = s_led_pins[i];

        GPIOE->MODER &= ~(3U << (pin * 2U));
        GPIOE->MODER |=  (1U << (pin * 2U));

        GPIOE->OTYPER &= ~(1U << pin);

        GPIOE->OSPEEDR &= ~(3U << (pin * 2U));
        GPIOE->OSPEEDR |=  (1U << (pin * 2U));

        GPIOE->PUPDR &= ~(3U << (pin * 2U));

        GPIOE->ODR &= ~(1U << pin);
    }
}

void LED_On(uint16_t pin)
{
    GPIOE->BSRR = (1U << pin);
}

void LED_Off(uint16_t pin)
{
    GPIOE->BRR = (1U << pin);
}

void LED_AllOff(void)
{
    uint32_t i;

    for (i = 0U; i < LED_COUNT; i++)
    {
        LED_Off(s_led_pins[i]);
    }
}

/* 4 heading quadrants using PE8..PE11 */
void LED_ShowHeading4(uint16_t heading_deg)
{
    uint8_t sector;

    heading_deg = (uint16_t)(heading_deg % 360U);
    sector = (uint8_t)(heading_deg / 90U);   /* 0..3 */

    LED_AllOff();
    LED_On(s_led_pins[sector]);
}
