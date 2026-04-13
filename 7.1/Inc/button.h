#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

typedef void (*button_callback_t)(void);

void button_init(button_callback_t callback);
bool button_is_pressed(void);
void button_poll(void);

#endif // BUTTON_H
