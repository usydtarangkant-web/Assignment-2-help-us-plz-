#ifndef BUTTON_H
#define BUTTON_H

typedef void (*button_callback_t)(void);

void Button_InitInterrupt(button_callback_t callback);

#endif
