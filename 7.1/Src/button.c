#include "button.h"
#include "gpio.h"

static button_callback_t button_callback = 0;
static bool last_button_state = false;

void button_init(button_callback_t callback)
{
    gpio_init_pin(GPIOA, 0, GPIO_PIN_MODE_INPUT);
    button_callback = callback;
    last_button_state = false;
}

bool button_is_pressed(void)
{
    return gpio_read_pin(GPIOA, 0);
}

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
