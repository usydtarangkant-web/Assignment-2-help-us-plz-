#include "led.h"
#include "button.h"
#include "systick.h"

/* Run when the button is pressed */
static void on_button_press(void)
{
    led_toggle(LED4);   /* Only change the target state */
}

int main(void)
{
    systick_init(8000);        /* 1 ms tick if clock is 8 MHz */
    led_init();                /* Initialise LEDs */
    led_set_min_interval(2000); /* Minimum 200 ms between LED changes */
    button_init(on_button_press); /* Set button callback */

    while (1)
    {
        button_poll();   /* Check the button */
        led_update();    /* Apply LED changes when time allows */
    }
}
