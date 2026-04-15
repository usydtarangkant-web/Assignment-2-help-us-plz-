#ifndef GPIO_H
#define GPIO_H

#include "stm32f303xc.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    GPIO_PIN_MODE_INPUT  = 0,
    GPIO_PIN_MODE_OUTPUT = 1
} gpio_mode_t;

void gpio_init_pin(GPIO_TypeDef *port, uint16_t pin, gpio_mode_t mode);
void gpio_write_pin(GPIO_TypeDef *port, uint16_t pin, bool value);
bool gpio_read_pin(GPIO_TypeDef *port, uint16_t pin);
void gpio_toggle_pin(GPIO_TypeDef *port, uint16_t pin);

#endif // GPIO_H
