#include "led.h"
#include "gpio.h"
#include "systick.h"

/* GPIO pin numbers for LED3 to LED10 */
static uint16_t led_pins[8] = {8, 9, 10, 11, 12, 13, 14, 15};

/* Current LED states */
static bool led_states[8] = {false, false, false, false, false, false, false, false};

/* Wanted LED states */
static bool led_targets[8] = {false, false, false, false, false, false, false, false};

/* Last update time for each LED */
static uint32_t led_last_update_ms[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* Minimum time between LED changes */
static uint32_t led_min_interval_ms = 200;

/* Initialise all LED pins */
void led_init(void)
{
    for (int i = 0; i < 8; i++)
    {
        gpio_init_pin(GPIOE, led_pins[i], GPIO_PIN_MODE_OUTPUT);
        gpio_write_pin(GPIOE, led_pins[i], false);

        led_states[i] = false;
        led_targets[i] = false;
        led_last_update_ms[i] = 0;
    }
}

/* Set the target state of one LED */
void led_set(led_t led, bool state)
{
    led_targets[led] = state;
}

/* Get the current state of one LED */
bool led_get(led_t led)
{
    return led_states[led];
}

/* Toggle the target state of one LED */
void led_toggle(led_t led)
{
    led_targets[led] = !led_targets[led];
}

/* Set the minimum time between LED updates */
void led_set_min_interval(uint32_t interval_ms)
{
    led_min_interval_ms = interval_ms;
}

/* Update LEDs when enough time has passed */
void led_update(void)
{
    uint32_t now = systick_get_ms();

    for (int i = 0; i < 8; i++)
    {
        if (led_states[i] != led_targets[i])
        {
            if ((now - led_last_update_ms[i]) >= led_min_interval_ms)
            {
                led_states[i] = led_targets[i];
                gpio_write_pin(GPIOE, led_pins[i], led_states[i]);
                led_last_update_ms[i] = now;
            }
        }
    }
}
