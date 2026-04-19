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
    LED_Init();
    Serial_Init();

    LED_On(8);   /* blue = init done */
    Serial_SendString("\r\nUSART1 debug started\r\n");
    Serial_SendString("Open screen at 115200 baud\r\n");

    while (1)
    {
        LED_Toggle(15);   /* green = main loop alive */
        Serial_SendString("hello from stm32 over usart1\r\n");
        SimpleDelay(800000);
    }
}
