//pwm.c (part c)
#include "pwm.h"
#include "stm32f303xc.h"
#include "timer.h"

static Timer_t   s_periodTimer;        //20ms periodic timer
static Timer_t   s_pulseTimer;         //One-shot pulse timer
static uint32_t  s_pulse_ticks;        //Current pulse in 100us ticks
//declare functions that are used before being defined
static void PeriodCallback(void);
static void PulseEndCallback(void);

//GPIO configuration (to control PA5 for pwm signal)
static void GPIO_ConfigureOutput(void)
{
    SERVO_GPIO_CLK_ENABLE(); //enable clock for GPIOA

    uint32_t pin = SERVO_GPIO_PIN; //PA5

    //Set PA5 pin mode in output mode (01)
    SERVO_GPIO_PORT->MODER &= ~(3U  << (pin * 2));
    SERVO_GPIO_PORT->MODER |=  (1U  << (pin * 2));
    //Set PA5 output type as push-pull (0) for high/low signals
    SERVO_GPIO_PORT->OTYPER &= ~(1U << pin);
    //set PA5 signal to medium speed to ensure no noise
    SERVO_GPIO_PORT->OSPEEDR &= ~(3U << (pin * 2));
    SERVO_GPIO_PORT->OSPEEDR |=  (1U << (pin * 2));
    //disable internal resistors
    SERVO_GPIO_PORT->PUPDR &= ~(3U << (pin * 2));
}

//Set PA5 high (1)
static inline void GPIO_PinHigh(void)
{
    SERVO_GPIO_PORT->BSRR = (1U << SERVO_GPIO_PIN);
}
//Set PA5 low (0)
static inline void GPIO_PinLow(void)
{
    SERVO_GPIO_PORT->BRR  = (1U << SERVO_GPIO_PIN);
}

//Servo initialisation
void Servo_Init(void)
{
    GPIO_ConfigureOutput();
    GPIO_PinLow();

    s_pulse_ticks = 15U;   //1.5ms = centre position as default (1000/1500)

    //Periodic 20ms timer starts immediately (part a)
    Timer_Init(&s_periodTimer, 20U, PeriodCallback);
}

//Servo position (pulse in us to updated pulse ticks)
void Servo_SetPulse(uint16_t pulse_us)
{
    //Clamp to valid range (1ms to 2ms: 1000us CW to 2000us CCW)
    if (pulse_us < SERVO_PULSE_CW_US) {
        pulse_us = SERVO_PULSE_CW_US;
    }
    if (pulse_us > SERVO_PULSE_CCW_US) {
        pulse_us = SERVO_PULSE_CCW_US;
    }

    //convert from us to 100us ticks
    s_pulse_ticks = pulse_us / 100U;
    //ensure ticks are between 10 and 20 (between 1ms to 2ms)
    if (s_pulse_ticks < 10U) {
        s_pulse_ticks = 10U;
    }
    if (s_pulse_ticks > 20U) {
        s_pulse_ticks = 20U;
    }
}

//For the servo ticks (called every systick: 100us)
void Servo_Tick(void)
{
    Timer_Tick(&s_periodTimer); //for the periodic timer
    Timer_Tick(&s_pulseTimer); //for the one-shot timer
}

//Periodic callback
static void PeriodCallback(void)
{
    GPIO_PinHigh(); //begin pulse
    Timer_OnceTicks(&s_pulseTimer, s_pulse_ticks, PulseEndCallback); //start one-shot to end pwm pulse
}

//After s_pulse_ticks (when PA5 is 0: low): end the pulse.
static void PulseEndCallback(void)
{
    GPIO_PinLow();                                     //end pulse
}
