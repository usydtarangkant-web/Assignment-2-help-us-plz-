#ifndef LED_H
#define LED_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    LED3 = 0,
    LED4,
    LED5,
    LED6,
    LED7,
    LED8,
    LED9,
    LED10
} led_t;

void led_init(void);
void led_set(led_t led, bool state);
bool led_get(led_t led);
void led_toggle(led_t led);

void led_set_min_interval(uint32_t interval_ms);

void led_update(void);

#endif // LED_H
