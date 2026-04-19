#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

/* Function type for button callback */
typedef void (*button_callback_t)(void);

/* Initialise the button and set the callback */
void button_init(button_callback_t callback);

/* Check if the button is pressed */
bool button_is_pressed(void);

/* Poll the button state */
void button_poll(void);

#endif // BUTTON_H
