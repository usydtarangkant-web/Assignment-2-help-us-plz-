#include "pwm.h"
#include "timer.h"

static Timer_t  s_periodTimer;
static Timer_t  s_pulseTimer;
static uint32_t s_pending_pulse_ticks;
static uint32_t s_active_pulse_ticks;

static void PeriodCallback(void);
static void PulseEndCallback(void);

static void GPIO_ConfigureOutput(void)
{
    uint32_t pin = SERVO_GPIO_PIN;

    SERVO_GPIO_CLK_ENABLE();

    SERVO_GPIO_PORT->MODER &= ~(3U << (pin * 2U));
    SERVO_GPIO_PORT->MODER |=  (1U << (pin * 2U));

    SERVO_GPIO_PORT->OTYPER &= ~(1U << pin);

    SERVO_GPIO_PORT->OSPEEDR &= ~(3U << (pin * 2U));
    SERVO_GPIO_PORT->OSPEEDR |=  (1U << (pin * 2U));

    SERVO_GPIO_PORT->PUPDR &= ~(3U << (pin * 2U));
}

static inline void GPIO_PinHigh(void)
{
    SERVO_GPIO_PORT->BSRR = (1U << SERVO_GPIO_PIN);
}

static inline void GPIO_PinLow(void)
{
    SERVO_GPIO_PORT->BRR = (1U << SERVO_GPIO_PIN);
}

void Servo_Init(void)
{
    GPIO_ConfigureOutput();
    GPIO_PinLow();

    s_pending_pulse_ticks = SERVO_PULSE_MID_US / 100U;
    s_active_pulse_ticks  = SERVO_PULSE_MID_US / 100U;

    Timer_Init(&s_periodTimer, 20U, PeriodCallback);

    Timer_InitTicks(&s_pulseTimer, s_active_pulse_ticks, PulseEndCallback);
    Timer_Stop(&s_pulseTimer);
}

void Servo_SetPulse(uint16_t pulse_us)
{
    uint32_t ticks;

    if (pulse_us < SERVO_PULSE_MIN_US)
    {
        pulse_us = SERVO_PULSE_MIN_US;
    }

    if (pulse_us > SERVO_PULSE_MAX_US)
    {
        pulse_us = SERVO_PULSE_MAX_US;
    }

    ticks = pulse_us / 100U;

    if (ticks < (SERVO_PULSE_MIN_US / 100U))
    {
        ticks = (SERVO_PULSE_MIN_US / 100U);
    }

    if (ticks > (SERVO_PULSE_MAX_US / 100U))
    {
        ticks = (SERVO_PULSE_MAX_US / 100U);
    }

    s_pending_pulse_ticks = ticks;
}

void Servo_Tick(void)
{
    Timer_Tick(&s_pulseTimer);
    Timer_Tick(&s_periodTimer);
}

static void PeriodCallback(void)
{
    s_active_pulse_ticks = s_pending_pulse_ticks;

    GPIO_PinHigh();
    Timer_OnceTicks(&s_pulseTimer, s_active_pulse_ticks, PulseEndCallback);
}

static void PulseEndCallback(void)
{
    GPIO_PinLow();
}
