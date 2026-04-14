#include "stm32f303xc.h"
#include "compass.h"

volatile uint32_t systemMillis = 0;


void SysTick_Handler(void)
{
    systemMillis++;
}


int main(void)
{
    CompassData compass;

    SysTick_Config(8000);

    compassInit();

    while (1)
    {
        compassRead(&compass);
    }
}
