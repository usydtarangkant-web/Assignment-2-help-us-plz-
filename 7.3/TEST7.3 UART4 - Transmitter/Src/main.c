#include "led.h"
#include "serial.h"
#include <stdint.h>

static void SimpleDelay(volatile uint32_t count)
{
    while (count > 0)
    {
        count--;
    }
}

int main(void)
{
    TestMessage txMsg;

    LED_Init();
    Serial_Init();

    LED_On(8);    /* blue = board started */
    Serial_SendString("\r\nTX board booted\r\n");
    Serial_SendString("USART1 debug active\r\n");
    Serial_SendString("UART4 packet transmit active\r\n");

    txMsg.sensorValue = 1023;
    txMsg.status = 1;
    txMsg.reserved = 0;

    while (1)
    {
        LED_Toggle(10);   /* orange toggles each send */

        Serial_SendString("TX sent packet\r\n");
        Serial_SendMsg(1, &txMsg, sizeof(TestMessage));

        SimpleDelay(900000);
    }
}
