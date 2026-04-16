//pwm.h (part c of task 7.2)
#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include "stm32f303xc.h"

//Pulse constants
#define SERVO_PULSE_CW_US      1000U   //Clockwise: 1.0 ms (1000 * 1)
#define SERVO_PULSE_CENTRE_US  1500U   //Centre: 1.5 ms (1000 * 1.5)
#define SERVO_PULSE_CCW_US     2000U   //Counter clockwise: 2.0 ms (1000 * 2)

//GPIO pin configuration
#define SERVO_GPIO_PORT        GPIOA
#define SERVO_GPIO_PIN         5U //set PA5 as pin for pwm signals
#define SERVO_GPIO_CLK_ENABLE() \
    (RCC->AHBENR |= RCC_AHBENR_GPIOAEN)

//state functions
void Servo_Init(void);
void Servo_SetPulse(uint16_t pulse_us);
void Servo_Tick(void);

#endif
