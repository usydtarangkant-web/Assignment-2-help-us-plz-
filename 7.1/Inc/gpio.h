#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>
#include <stdint.h>

typedef struct GPIO_TypeDef GPIO_TypeDef;

typedef enum {
    GPIO_PIN_MODE_INPUT  = 0,
    GPIO_PIN_MODE_OUTPUT = 1
} gpio_mode_t;

// GPIO port base addresses
#define GPIOA_BASE 0x48000000UL
#define GPIOB_BASE 0x48000400UL
#define GPIOC_BASE 0x48000800UL
#define GPIOD_BASE 0x48000C00UL
#define GPIOE_BASE 0x48001000UL
#define GPIOF_BASE 0x48001400UL

// GPIO port pointers
#define GPIOA ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC ((GPIO_TypeDef *)GPIOC_BASE)
#define GPIOD ((GPIO_TypeDef *)GPIOD_BASE)
#define GPIOE ((GPIO_TypeDef *)GPIOE_BASE)
#define GPIOF ((GPIO_TypeDef *)GPIOF_BASE)

void gpio_init_pin(GPIO_TypeDef *port, uint16_t pin, gpio_mode_t mode);
void gpio_write_pin(GPIO_TypeDef *port, uint16_t pin, bool value);
bool gpio_read_pin(GPIO_TypeDef *port, uint16_t pin);
void gpio_toggle_pin(GPIO_TypeDef *port, uint16_t pin);

#endif // GPIO_H
