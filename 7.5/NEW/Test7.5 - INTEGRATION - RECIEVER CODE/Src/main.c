#include "stm32f303xc.h"
#include "serial.h"
#include "integration_protocol.h"
#include "led.h"
#include "pwm.h"
#include "timer.h"
#include <stdio.h>

static volatile uint8_t g_callback_hit = 0U;
static volatile uint8_t g_packet_valid = 0U;

static volatile IntegrationMessage_t g_latest_msg;
static volatile uint8_t g_latest_msg_type = 0U;
static volatile uint16_t g_latest_length = 0U;

/* Called when a full valid packet arrives */
static void ReceiverPacketCallback(uint8_t msgType, uint8_t *payload, uint16_t length)
{
    IntegrationMessage_t *msg;

    g_callback_hit = 1U;
    g_packet_valid = 0U;
    g_latest_msg_type = msgType;
    g_latest_length = length;

    if ((msgType == MSG_TYPE_COMPASS_DATA) &&
        (length == sizeof(IntegrationMessage_t)))
    {
        msg = (IntegrationMessage_t *)payload;
        g_latest_msg = *msg;
        g_packet_valid = 1U;
    }

    /* BREAKPOINT: receiver callback got one full packet */
}

int main(void)
{
    uint32_t last_heartbeat_ms = 0U;
    char text[160];

    Timer_InitMs();
    LED_Init();
    PWM_InitServoPA5();
    Serial_Init();
    Serial_SetRxCallback(ReceiverPacketCallback);

    /* Boot text */
    Serial_SendString("\r\nRX integration board booted\r\n");
    Serial_SendString("RX serial + servo + LED heading active\r\n");

    /* Start with all LEDs off */
    LED_AllOff();

    while (1)
    {
        /* Let serial module hand full packets to callback */
        Serial_Task();

        /* Heartbeat on PE15 */
        if ((Timer_GetMillis() - last_heartbeat_ms) >= 500U)
        {
            last_heartbeat_ms = Timer_GetMillis();
            LED_Toggle(15U);
        }

        if (g_callback_hit != 0U)
        {
            g_callback_hit = 0U;

            if (g_packet_valid != 0U)
            {
                /* Print exactly what receiver decoded */
                snprintf(text, sizeof(text),
                         "RX got: heading=%u mode=%u mag=(%d,%d,%d) time=%lu\r\n",
                         g_latest_msg.heading_deg,
                         g_latest_msg.display_mode,
                         g_latest_msg.mag_x,
                         g_latest_msg.mag_y,
                         g_latest_msg.mag_z,
                         (unsigned long)g_latest_msg.timestamp_ms);
                Serial_SendString(text);

                if (g_latest_msg.display_mode == DISPLAY_MODE_SERVO_ONLY)
                {
                    /* Servo mode */
                    PWM_SetFromHeading(g_latest_msg.heading_deg);

                    Serial_SendString("RX action: SERVO updated from heading\r\n");

                    /* In servo mode, turn LED heading display off */
                    LED_AllOff();
                }
                else
                {
                    /* LED heading mode */
                    LED_ShowHeading8(g_latest_msg.heading_deg);

                    Serial_SendString("RX action: LED heading updated\r\n");
                }
            }
            else
            {
                Serial_SendString("RX bad packet type or size\r\n");
                LED_On(9);   /* red style error indicator */
            }
        }

        if (dbg_rxOverflow)
        {
            Serial_SendString("RX overflow error\r\n");
            LED_On(9);
        }
    }
}
