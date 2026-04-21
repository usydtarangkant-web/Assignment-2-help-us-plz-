#include "stm32f303xc.h"
#include "serial.h"
#include "compass.h"
#include "button.h"
#include "integration_protocol.h"
#include <stdio.h>

/* Timing counters updated by SysTick */
volatile uint32_t tick_100us = 0U;
volatile uint32_t tick_ms = 0U;

/* Current display mode to send to receiver */
static volatile uint8_t g_display_mode = DISPLAY_MODE_SERVO_ONLY;

/* Set when button is pressed so main loop can print debug text */
static volatile uint8_t g_button_event = 0U;

/* Small debug LEDs on board */
static void DebugLED_Init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;

    /* PE12, PE13, PE14, PE15 as outputs */
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

/* SysTick every 100 us */
static void Board1_SysTickInit(void)
{
    SysTick_Config(8000000U / 10000U);
}

/* Called when button is pressed */
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

    /* main loop will print a message */
    g_button_event = 1U;

    /* PE12 toggles on each button press */
    DebugLED_Toggle(12U);
}

/* Convert float heading to uint16 for packet */
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

/* SysTick interrupt updates time counters */
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
    char text[140];
    uint16_t heading_to_send;

    /* Enable FPU because compass uses atan2f */
    SCB->CPACR |= (0xFUL << 20);

    Board1_SysTickInit();
    DebugLED_Init();
    Serial_Init();
    compassInit();
    Button_InitInterrupt(ButtonPressedCallback);

    Serial_SendString("\r\nTX integration board booted\r\n");
    Serial_SendString("TX compass + button + serial active\r\n");

    while (1)
    {
        /* Keep serial module running */
        Serial_Task();

        /* Print button event in main loop, not inside interrupt */
        if (g_button_event != 0U)
        {
            g_button_event = 0U;

            if (g_display_mode == DISPLAY_MODE_SERVO_ONLY)
            {
                Serial_SendString("TX button pressed -> mode = SERVO ONLY\r\n");
            }
            else
            {
                Serial_SendString("TX button pressed -> mode = LED HEADING\r\n");
            }
        }

        /* Heartbeat LED every 500 ms */
        if ((tick_ms - last_heartbeat_ms) >= 500U)
        {
            last_heartbeat_ms = tick_ms;
            DebugLED_Toggle(15U);
        }

        /* Send one integration packet every 100 ms */
        if ((tick_ms - last_send_ms) >= 100U)
        {
            last_send_ms = tick_ms;

            if (compassRead(&compass_data) == 0)
            {
                heading_to_send = HeadingFloatToUint16(compass_data.heading);

                /* Fill packet */
                tx_msg.mag_x = compass_data.x;
                tx_msg.mag_y = compass_data.y;
                tx_msg.mag_z = compass_data.z;
                tx_msg.heading_deg = heading_to_send;
                tx_msg.timestamp_ms = compass_data.timestamp;
                tx_msg.display_mode = g_display_mode;
                tx_msg.reserved[0] = 0U;
                tx_msg.reserved[1] = 0U;
                tx_msg.reserved[2] = 0U;

                /* Send packet to receiver board */
                Serial_SendMsg(MSG_TYPE_COMPASS_DATA, &tx_msg, sizeof(tx_msg));

                /* PE14 toggles when packet is sent */
                DebugLED_Toggle(14U);

                /* PE13 off means compass read was good */
                DebugLED_Off(13U);

                /* Readable debug text to Mac */
                snprintf(text, sizeof(text),
                         "TX send: heading=%u mode=%u mag=(%d,%d,%d) time=%lu -> RX should show %s\r\n",
                         tx_msg.heading_deg,
                         tx_msg.display_mode,
                         tx_msg.mag_x,
                         tx_msg.mag_y,
                         tx_msg.mag_z,
                         (unsigned long)tx_msg.timestamp_ms,
                         (tx_msg.display_mode == DISPLAY_MODE_SERVO_ONLY) ? "SERVO" : "LED");
                Serial_SendString(text);

                /* Simple heading text for tutor demo */
                if ((tx_msg.heading_deg >= 315U) || (tx_msg.heading_deg < 45U))
                {
                    Serial_SendString("TX heading zone: NORTH\r\n");
                }
                else if (tx_msg.heading_deg < 135U)
                {
                    Serial_SendString("TX heading zone: EAST\r\n");
                }
                else if (tx_msg.heading_deg < 225U)
                {
                    Serial_SendString("TX heading zone: SOUTH\r\n");
                }
                else
                {
                    Serial_SendString("TX heading zone: WEST\r\n");
                }

                /* BREAKPOINT: full integration packet just sent */
            }
            else
            {
                /* PE13 on means compass read error */
                DebugLED_On(13U);
                Serial_SendString("TX compass read error\r\n");
            }
        }
    }
}
