#include "gpio.h"

#define RCC_BASE   0x40021000UL

struct GPIO_TypeDef {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
};

typedef struct {
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} RCC_TypeDef;

#define RCC   ((RCC_TypeDef *)RCC_BASE)

#define RCC_AHBENR_GPIOAEN (1UL << 17)
#define RCC_AHBENR_GPIOBEN (1UL << 18)
#define RCC_AHBENR_GPIOCEN (1UL << 19)
#define RCC_AHBENR_GPIODEN (1UL << 20)
#define RCC_AHBENR_GPIOEEN (1UL << 21)
#define RCC_AHBENR_GPIOFEN (1UL << 22)

static void gpio_enable_clock(GPIO_TypeDef *port)
{
    if (port == GPIOA)      RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    else if (port == GPIOB) RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    else if (port == GPIOC) RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    else if (port == GPIOD) RCC->AHBENR |= RCC_AHBENR_GPIODEN;
    else if (port == GPIOE) RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
    else if (port == GPIOF) RCC->AHBENR |= RCC_AHBENR_GPIOFEN;
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
        port->ODR |= (1U << pin);
    else
        port->ODR &= ~(1U << pin);
}

bool gpio_read_pin(GPIO_TypeDef *port, uint16_t pin)
{
    return ((port->IDR & (1U << pin)) != 0U);
}

void gpio_toggle_pin(GPIO_TypeDef *port, uint16_t pin)
{
    port->ODR ^= (1U << pin);
}
