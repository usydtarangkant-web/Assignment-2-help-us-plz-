#include "stm32f303xc.h"      // Include STM32 register definitions
#include "compass.h"          // Include compass sensor functions and structure
#include "compass_led.h"      // Include optional LED display functions


/* Uncomment this line to enable LED heading display */
//#define USE_LEDS


/* Global millisecond timer variable */
volatile uint32_t tick_ms = 0;


/* Global structure used to store compass data */
CompassData compass;


/* SysTick interrupt runs every 1 millisecond */
void SysTick_Handler(void)
{
    tick_ms++;                // Increase timer count
}


/* Main program */
int main(void)
{
    /* Enable floating point unit for atan2 calculations */
    SCB->CPACR |= (0xFUL << 20);

    /* Configure SysTick to interrupt every 1 ms */
    SysTick_Config(8000);

    /* Initialise onboard compass sensor */
    compassInit();


#ifdef USE_LEDS
    /* Initialise LEDs if enabled */
    compassLEDInit();
#endif


    while (1)
    {
        /* Read latest compass values into structure */
        compassRead(&compass);


#ifdef USE_LEDS
        /* Display heading on LEDs if enabled */
        compassLEDDisplay(&compass);
#endif


        /* Small delay to slow update rate */
        for (volatile int i = 0; i < 100000; i++);
    }
}
