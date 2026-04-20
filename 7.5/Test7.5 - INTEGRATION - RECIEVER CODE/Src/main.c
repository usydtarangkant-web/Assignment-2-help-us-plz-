#include "stm32f303xc.h"
#include "serial.h"
#include "pwm.h"
#include "led.h"
#include "integration_protocol.h"
#include <string.h>

#define HEADING_DEADBAND_DEG  2U
#define SERVO_MIN_PULSE_US    700U
#define SERVO_MAX_PULSE_US    2300U

volatile uint32_t tick_100us = 0U;
volatile uint32_t tick_ms = 0U;

static volatile uint8_t g_new_msg = 0U;
static IntegrationMessage_t g_latest_msg;

static void DebugLED_Init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;

    /* PE12 PE13 PE14 PE15 are debug LEDs only */
    GPIOE->MODER &= ~(3U << (12U * 2U));
    GPIOE->MODER &= ~(3U << (13U * 2U));
    GPIOE->MODER &= ~(3U << (14U * 2U));
    GPIOE->MODER &= ~(3U << (15U * 2U));

    GPIOE->MODER |=  (1U << (12U * 2U));
    GPIOE->MODER |=  (1U << (13U * 2U));
    GPIOE->MODER |=  (1U << (14U * 2U));
    GPIOE->MODER |=  (1U << (15U * 2U));

    GPIOE->ODR &= ~((1U << 12U) | (1U << 13U) | (1U << 14U) | (1U << 15U));
}

static void DebugLED_On(uint32_t pin)
{
    GPIOE->BSRR = (1U << pin);
}

static void DebugLED_Off(uint32_t pin)
{
    GPIOE->BRR = (1U << pin);
}

static void DebugLED_Toggle(uint32_t pin)
{
    GPIOE->ODR ^= (1U << pin);
}

static void Board2_SysTickInit(void)
{
    SysTick_Config(8000000U / 10000U);
}

static void Board2_RxCallback(uint8_t msgType, uint8_t *payload, uint16_t length)
{
    if ((msgType == MSG_TYPE_COMPASS_DATA) && (length == sizeof(IntegrationMessage_t)))
    {
        memcpy((void *)&g_latest_msg, (const void *)payload, sizeof(IntegrationMessage_t));
        g_new_msg = 1U;
    }
}

static uint16_t HeadingToServoPulseUs(uint16_t heading_deg)
{
    uint32_t span_us;

    if (heading_deg > 359U)
    {
        heading_deg = 359U;
    }

    span_us = (uint32_t)(SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US);

    return (uint16_t)(SERVO_MIN_PULSE_US + ((uint32_t)heading_deg * span_us) / 359U);
}

static uint16_t HeadingCircularDiff(uint16_t a, uint16_t b)
{
    uint16_t diff;

    if (a >= b)
    {
        diff = a - b;
    }
    else
    {
        diff = b - a;
    }

    if (diff > 180U)
    {
        diff = 360U - diff;
    }

    return diff;
}

void SysTick_Handler(void)
{
    static uint8_t sub_count = 0U;

    tick_100us++;
    sub_count++;

    Servo_Tick();

    if (sub_count >= 10U)
    {
        sub_count = 0U;
        tick_ms++;
    }
}

int main(void)
{
    IntegrationMessage_t local_msg;
    uint16_t servo_pulse_us;
    uint16_t heading_diff;
    uint32_t last_heartbeat_ms = 0U;
    uint16_t last_heading_deg = 0xFFFFU;

    Board2_SysTickInit();
    DebugLED_Init();
    LED_Init();
    Servo_Init();
    Serial_Init();
    Serial_SetRxCallback(Board2_RxCallback);

    while (1)
    {
        Serial_Task();

        /* LD6 = heartbeat */
        if ((tick_ms - last_heartbeat_ms) >= 500U)
        {
            last_heartbeat_ms = tick_ms;
            DebugLED_Toggle(15U);
        }

        /* LD10 = current packet error state only */
        if ((dbg_packetReady != 0U) && (dbg_packetValid == 0U))
        {
            DebugLED_On(13U);
        }
        else
        {
            DebugLED_Off(13U);
        }

        if (g_new_msg != 0U)
        {
            g_new_msg = 0U;
            local_msg = g_latest_msg;

            /* LD8 = valid packet received */
            DebugLED_Toggle(14U);

            if (last_heading_deg == 0xFFFFU)
            {
                heading_diff = 360U;
            }
            else
            {
                heading_diff = HeadingCircularDiff(local_msg.heading_deg, last_heading_deg);
            }

            if ((last_heading_deg == 0xFFFFU) || (heading_diff >= HEADING_DEADBAND_DEG))
            {
                servo_pulse_us = HeadingToServoPulseUs(local_msg.heading_deg);
                Servo_SetPulse(servo_pulse_us);
                last_heading_deg = local_msg.heading_deg;
            }

            if (local_msg.display_mode == DISPLAY_MODE_LED_HEADING)
            {
                /* LD9 = LED mode active */
                DebugLED_On(12U);
                LED_ShowHeading4(local_msg.heading_deg);
            }
            else
            {
                DebugLED_Off(12U);
                LED_AllOff();
            }
        }
    }
}
