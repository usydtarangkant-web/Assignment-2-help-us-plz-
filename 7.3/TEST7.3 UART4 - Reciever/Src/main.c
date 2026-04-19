#include "led.h"
#include "serial.h"
#include <stdint.h>

static volatile uint8_t callbackHit = 0;
static volatile uint8_t lastMsgType = 0;
static volatile uint16_t lastMsgLength = 0;
static volatile uint16_t lastSensorValue = 0;
static volatile uint8_t lastStatus = 0;

void MySerialRxHandler(uint8_t msgType, uint8_t *payload, uint16_t length)
{
    TestMessage *msg;

    callbackHit = 1;
    lastMsgType = msgType;
    lastMsgLength = length;

    if ((msgType == 1) && (length == sizeof(TestMessage)))
    {
        msg = (TestMessage *)payload;
        lastSensorValue = msg->sensorValue;
        lastStatus = msg->status;

        if ((lastSensorValue == 1023) && (lastStatus == 1))
        {
            LED_On(15);   /* green = correct packet */
            LED_Off(9);
            Serial_SendString("RX valid packet\r\n");
        }
        else
        {
            LED_On(9);    /* red = wrong contents */
            LED_Off(15);
            Serial_SendString("RX wrong payload\r\n");
        }
    }
    else
    {
        LED_On(9);        /* red = wrong type or length */
        LED_Off(15);
        Serial_SendString("RX wrong type/length\r\n");
    }
}

int main(void)
{
    LED_Init();
    Serial_Init();
    Serial_SetRxCallback(MySerialRxHandler);

    LED_On(8);   /* blue = board started */
    Serial_SendString("\r\nRX board booted\r\n");
    Serial_SendString("USART1 debug active\r\n");
    Serial_SendString("Waiting for UART4 packet...\r\n");

    while (1)
    {
        Serial_Task();

        if (callbackHit)
        {
            LED_Toggle(10);   /* orange toggles whenever callback happened */
            callbackHit = 0;
        }

        if (dbg_rxOverflow || dbg_txOverflow)
        {
            LED_On(9);
            Serial_SendString("RX/TX overflow error\r\n");

            while (1)
            {
            }
        }
    }
}
