#ifndef LED_H
#define LED_H

#include "stm32f303xc.h"
#include <stdint.h>

void LED_Init(void);
void LED_AllOff(void);
void LED_On(uint16_t pin);
void LED_Off(uint16_t pin);
void LED_Toggle(uint16_t pin);
void LED_ShowHeading8(uint16_t heading_deg);

#endif
