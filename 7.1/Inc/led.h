#ifndef LED_H
#define LED_H

#include <stdbool.h>
#include <stdint.h>

/* LEDs on the STM32F3 Discovery board */
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

/* Initialise all LEDs */
void led_init(void);

/* Turn one LED on or off */
void led_set(led_t led, bool state);

/* Get the current LED state */
bool led_get(led_t led);

/* Change the LED state */
void led_toggle(led_t led);

/* Set the minimum time between LED changes */
void led_set_min_interval(uint32_t interval_ms);

/* Update LEDs if timing control is used */
void led_update(void);

#endif // LED_H
