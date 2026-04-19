#include "stm32f303xc.h"
#include "compass.h"
#include "compass_led.h"


/* Uncomment to enable LED compass display */
//#define USE_LEDS


/* Global millisecond timer */
volatile uint32_t tick_ms = 0;


/* Global compass structure */
CompassData compass;


/* SysTick interrupt every 1 ms */
void SysTick_Handler(void)
{
    tick_ms++;
}


int main(void)
{
    /* Enable floating point unit */
    SCB->CPACR |= (0xFUL << 20);

    /* Configure SysTick for 1 ms interrupts */
    SysTick_Config(8000);

    /* Initialise compass */
    compassInit();


#ifdef USE_LEDS
    compassLEDInit();
#endif


    while (1)
    {
        /* Update compass data */
        compassRead(&compass);


#ifdef USE_LEDS
        compassLEDDisplay(&compass);
#endif


        /* Small delay */
        for (volatile int i = 0; i < 100000; i++);
    }
}
