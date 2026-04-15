#include "compass_led.h"


void compassLEDInit(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;

    for (int i = 8; i <= 15; i++)
    {
        GPIOE->MODER &= ~(0x3UL << (i * 2));
        GPIOE->MODER |=  (0x1UL << (i * 2));
    }
}


void compassLEDDisplay(CompassData* data)
{
    GPIOE->ODR &= ~(0xFF << 8);

    if (data->heading < 45)
        GPIOE->ODR |= (1 << 8);

    else if (data->heading < 90)
        GPIOE->ODR |= (1 << 9);

    else if (data->heading < 135)
        GPIOE->ODR |= (1 << 10);

    else if (data->heading < 180)
        GPIOE->ODR |= (1 << 11);

    else if (data->heading < 225)
        GPIOE->ODR |= (1 << 12);

    else if (data->heading < 270)
        GPIOE->ODR |= (1 << 13);

    else if (data->heading < 315)
        GPIOE->ODR |= (1 << 14);

    else
        GPIOE->ODR |= (1 << 15);
}
