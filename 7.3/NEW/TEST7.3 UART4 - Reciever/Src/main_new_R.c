#include "serial.h"
#include "led.h"
#include <stdio.h>

/* Match the transmitter message type */
#define MSG_TYPE_TEST  1

static volatile uint8_t callbackHit = 0;
static volatile uint8_t packetValid = 0;
static volatile uint8_t lastMsgType = 0;
static volatile uint16_t lastLength = 0;
static volatile uint16_t lastSensorValue = 0;
static volatile uint8_t lastStatus = 0;
static volatile uint8_t lastReserved = 0;

static void MySerialRxHandler(uint8_t msgType, uint8_t *payload, uint16_t length)
{
    TestMessage *msg;

    callbackHit = 1;
    packetValid = 0;
    lastMsgType = msgType;
    lastLength = length;

    LED_Off(9);   /* clear old error */
    LED_Off(15);  /* clear old valid */

    /* Receiver should validate type + length,
       not one exact magic value */
    if ((msgType == MSG_TYPE_TEST) && (length == sizeof(TestMessage)))
    {
        msg = (TestMessage *)payload;

        lastSensorValue = msg->sensorValue;
        lastStatus = msg->status;
        lastReserved = msg->reserved;

        packetValid = 1;
    }
    else
    {
        LED_On(9);
    }
}

int main(void)
{
    char text[100];

    LED_Init();
    Serial_Init();
    Serial_SetRxCallback(MySerialRxHandler);

    /* Demo LED meaning
       blue   = booted
       green  = valid packet decoded
       orange = callback happened
       red    = RX error */
    LED_On(8);

    Serial_SendString("\r\nRX board booted\r\n");
    Serial_SendString("RX debug active\r\n");

    while (1)
    {
        /* Let the serial module hand complete packets to callback */
        Serial_Task();

        if (callbackHit)
        {
            callbackHit = 0;
            LED_Toggle(10);

            if (packetValid)
            {
                LED_On(15);

                snprintf(text, sizeof(text),
                         "RX got: type=%u value=%u status=%u reserved=%u\r\n",
                         lastMsgType,
                         lastSensorValue,
                         lastStatus,
                         lastReserved);
                Serial_SendString(text);
            }
            else
            {
                LED_On(9);
                Serial_SendString("RX bad packet type or size\r\n");
            }

            /* BREAKPOINT: receiver callback finished */
        }

        if (dbg_rxOverflow)
        {
            LED_On(9);
            Serial_SendString("RX overflow error\r\n");

            while (1)
            {
            }
        }
    }
}
