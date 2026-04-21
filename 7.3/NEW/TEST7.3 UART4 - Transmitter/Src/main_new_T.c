#include "serial.h"
#include "led.h"
#include <stdio.h>

/* One simple message type for this demo */
#define MSG_TYPE_TEST  1

int main(void)
{
    TestMessage txMsg;
    char text[100];
    uint16_t counter = 0;

    LED_Init();
    Serial_Init();

    /* Demo LED meaning
       blue   = booted
       green  = running
       orange = packet sent
       red    = TX error */
    LED_On(8);
    LED_On(15);

    Serial_SendString("\r\nTX board booted\r\n");
    Serial_SendString("TX debug active\r\n");

    txMsg.sensorValue = 0;
    txMsg.status = 1;
    txMsg.reserved = 0;

    while (1)
    {
        /* Change these to prove the receiver follows the new values */
        txMsg.sensorValue = counter;
        txMsg.status ^= 1U;
        txMsg.reserved = 0;

        /* Send real packet to the other STM32 */
        Serial_SendMsg(MSG_TYPE_TEST, &txMsg, sizeof(TestMessage));

        /* Send readable debug text to the PC */
        snprintf(text, sizeof(text),
                 "TX sent: type=%u value=%u status=%u reserved=%u\r\n",
                 MSG_TYPE_TEST,
                 txMsg.sensorValue,
                 txMsg.status,
                 txMsg.reserved);
        Serial_SendString(text);

        LED_Toggle(10);

        /* BREAKPOINT: transmitter just queued one packet */

        if (dbg_txOverflow)
        {
            LED_On(9);
            LED_Off(15);
            Serial_SendString("TX overflow error\r\n");

            while (1)
            {
            }
        }

        counter++;

        /* Bigger = slower */
        LED_Delay(800000);
    }
}
