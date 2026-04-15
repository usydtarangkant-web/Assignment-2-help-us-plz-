#include "gpio.h"

static void gpio_enable_clock(GPIO_TypeDef *port)
{
    if (port == GPIOA)
    {
        RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    }
    else if (port == GPIOB)
    {
        RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    }
    else if (port == GPIOC)
    {
        RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    }
    else if (port == GPIOD)
    {
        RCC->AHBENR |= RCC_AHBENR_GPIODEN;
    }
    else if (port == GPIOE)
    {
        RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
    }
    else if (port == GPIOF)
    {
        RCC->AHBENR |= RCC_AHBENR_GPIOFEN;
    }
}

void gpio_init_pin(GPIO_TypeDef *port, uint16_t pin, gpio_mode_t mode)
{
    gpio_enable_clock(port);

    port->MODER &= ~(0x3U << (pin * 2));

    if (mode == GPIO_PIN_MODE_OUTPUT)
    {
        port->MODER |= (0x1U << (pin * 2));
    }
}

void gpio_write_pin(GPIO_TypeDef *port, uint16_t pin, bool value)
{
    if (value)
    {
        port->BSRR = (1U << pin);
    }
    else
    {
        port->BSRR = (1U << (pin + 16));
    }
}

bool gpio_read_pin(GPIO_TypeDef *port, uint16_t pin)
{
    return ((port->IDR & (1U << pin)) != 0U);
}

void gpio_toggle_pin(GPIO_TypeDef *port, uint16_t pin)
{
    port->ODR ^= (1U << pin);
}
