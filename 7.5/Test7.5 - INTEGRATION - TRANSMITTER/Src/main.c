#include "stm32f303xc.h"
#include "serial.h"
#include "compass.h"
#include "button.h"
#include "integration_protocol.h"

volatile uint32_t tick_100us = 0U;
volatile uint32_t tick_ms = 0U;

static volatile uint8_t g_display_mode = DISPLAY_MODE_SERVO_ONLY;

static void DebugLED_Init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;

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

static void Board1_SysTickInit(void)
{
    SysTick_Config(8000000U / 10000U);
}

static void ButtonPressedCallback(void)
{
    if (g_display_mode == DISPLAY_MODE_SERVO_ONLY)
    {
        g_display_mode = DISPLAY_MODE_LED_HEADING;
    }
    else
    {
        g_display_mode = DISPLAY_MODE_SERVO_ONLY;
    }

    DebugLED_Toggle(12U);
}

static uint16_t HeadingFloatToUint16(float heading)
{
    if (heading < 0.0f)
    {
        heading = 0.0f;
    }

    if (heading >= 360.0f)
    {
        heading -= 360.0f;
    }

    if (heading > 359.0f)
    {
        heading = 359.0f;
    }

    return (uint16_t)heading;
}

void SysTick_Handler(void)
{
    static uint8_t sub_count = 0U;

    tick_100us++;
    sub_count++;

    if (sub_count >= 10U)
    {
        sub_count = 0U;
        tick_ms++;
    }
}

int main(void)
{
    CompassData compass_data;
    IntegrationMessage_t tx_msg;
    uint32_t last_send_ms = 0U;
    uint32_t last_heartbeat_ms = 0U;

    /* enable FPU because compass.c uses atan2f */
    SCB->CPACR |= (0xFUL << 20);

    Board1_SysTickInit();
    DebugLED_Init();
    Serial_Init();
    compassInit();
    Button_InitInterrupt(ButtonPressedCallback);

    while (1)
    {
        Serial_Task();

        if ((tick_ms - last_heartbeat_ms) >= 500U)
        {
            last_heartbeat_ms = tick_ms;
            DebugLED_Toggle(15U);
        }

        if ((tick_ms - last_send_ms) >= 100U)
        {
            last_send_ms = tick_ms;

            if (compassRead(&compass_data) == 0)
            {
                tx_msg.mag_x = compass_data.x;
                tx_msg.mag_y = compass_data.y;
                tx_msg.mag_z = compass_data.z;
                tx_msg.heading_deg = HeadingFloatToUint16(compass_data.heading);
                tx_msg.timestamp_ms = compass_data.timestamp;
                tx_msg.display_mode = g_display_mode;
                tx_msg.reserved[0] = 0U;
                tx_msg.reserved[1] = 0U;
                tx_msg.reserved[2] = 0U;

                Serial_SendMsg(MSG_TYPE_COMPASS_DATA, &tx_msg, sizeof(tx_msg));

                DebugLED_Toggle(14U);
                DebugLED_Off(13U);
            }
            else
            {
                DebugLED_On(13U);
            }
        }
    }
}
