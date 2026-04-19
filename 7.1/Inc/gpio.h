#ifndef GPIO_H
#define GPIO_H

#include "stm32f303xc.h"
#include <stdbool.h>
#include <stdint.h>

/* GPIO pin mode */
typedef enum {
    GPIO_PIN_MODE_INPUT  = 0,
    GPIO_PIN_MODE_OUTPUT = 1
} gpio_mode_t;

/* Initialise one GPIO pin */
void gpio_init_pin(GPIO_TypeDef *port, uint16_t pin, gpio_mode_t mode);

/* Write a value to one GPIO pin */
void gpio_write_pin(GPIO_TypeDef *port, uint16_t pin, bool value);

/* Read the value of one GPIO pin */
bool gpio_read_pin(GPIO_TypeDef *port, uint16_t pin);

/* Toggle one GPIO pin */
void gpio_toggle_pin(GPIO_TypeDef *port, uint16_t pin);

#endif // GPIO_H
