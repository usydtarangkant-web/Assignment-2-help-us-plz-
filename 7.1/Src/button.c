#include "button.h"
#include "gpio.h"

/* Stored callback function */
static button_callback_t button_callback = 0;

/* Last button state */
static bool last_button_state = false;

/* Initialise the button and save the callback */
void button_init(button_callback_t callback)
{
    gpio_init_pin(GPIOA, 0, GPIO_PIN_MODE_INPUT);
    button_callback = callback;
    last_button_state = false;
}

/* Check if the button is pressed */
bool button_is_pressed(void)
{
    return gpio_read_pin(GPIOA, 0);
}

/* Poll the button and call the callback on a new press */
void button_poll(void)
{
    bool current_state = button_is_pressed();

    if ((current_state == true) && (last_button_state == false))
    {
        if (button_callback != 0)
        {
            button_callback();
        }
    }

    last_button_state = current_state;
}
