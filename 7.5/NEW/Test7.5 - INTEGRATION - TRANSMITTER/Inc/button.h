#ifndef BUTTON_H
#define BUTTON_H

/* Function pointer for button callback */
typedef void (*button_callback_t)(void);

/* Set up PA0 button interrupt */
void Button_InitInterrupt(button_callback_t callback);

#endif
