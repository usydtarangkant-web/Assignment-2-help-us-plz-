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
            LED_On(15);
        }
        else
        {
            LED_On(9);
        }
    }
    else
    {
        LED_On(9);
    }
}

int main(void)
{
    TestMessage txMsg;
    uint8_t sentOnce = 0;
    uint32_t timeout = 0;

    LED_Init();
    LED_On(8);

    Serial_Init();
    LED_On(10);
    Serial_SetRxCallback(MySerialRxHandler);

    txMsg.sensorValue = 1023;
    txMsg.status = 1;
    txMsg.reserved = 0;

    while (1)
    {
        if (sentOnce == 0)
        {
            Serial_SendMsg(1, &txMsg, sizeof(TestMessage));
            sentOnce = 1;
        }

        Serial_Task();

        if (callbackHit)
        {
            while (1)
            {
            }
        }

        timeout++;
        if (timeout > 4000000)
        {
            LED_On(9);
            while (1)
            {
            }
        }

        if (dbg_rxOverflow || dbg_txOverflow)
        {
            LED_On(9);
            while (1)
            {
            }
        }
    }
}
