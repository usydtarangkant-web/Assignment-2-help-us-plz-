#ifndef PWM_H
#define PWM_H

#include "stm32f303xc.h"
#include <stdint.h>

#define SERVO_GPIO_PORT         GPIOC
#define SERVO_GPIO_PIN          8U
#define SERVO_GPIO_CLK_ENABLE() (RCC->AHBENR |= RCC_AHBENR_GPIOCEN)

#define SERVO_PULSE_MIN_US      700U
#define SERVO_PULSE_MID_US      1500U
#define SERVO_PULSE_MAX_US      2300U

void Servo_Init(void);
void Servo_SetPulse(uint16_t pulse_us);
void Servo_Tick(void);

#endif
