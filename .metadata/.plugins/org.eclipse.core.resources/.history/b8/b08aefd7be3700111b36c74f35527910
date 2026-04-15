#include "compass.h"
#include <math.h>

#define COMPASS_ADDR 0x1E

volatile extern uint32_t systemMillis;


void i2cWriteRegister(uint8_t reg, uint8_t value)
{
    I2C1->CR2 = (COMPASS_ADDR << 1) | (2 << 16) | I2C_CR2_START | I2C_CR2_AUTOEND;

    while (!(I2C1->ISR & I2C_ISR_TXIS));
    I2C1->TXDR = reg;

    while (!(I2C1->ISR & I2C_ISR_TXIS));
    I2C1->TXDR = value;

    while (!(I2C1->ISR & I2C_ISR_STOPF));

    I2C1->ICR |= I2C_ICR_STOPCF;
}


void i2cReadRegisters(uint8_t reg, uint8_t* buffer, uint8_t length)
{
    I2C1->CR2 = (COMPASS_ADDR << 1) | (1 << 16) | I2C_CR2_START;

    while (!(I2C1->ISR & I2C_ISR_TXIS));
    I2C1->TXDR = reg;

    while (!(I2C1->ISR & I2C_ISR_TC));

    I2C1->CR2 = (COMPASS_ADDR << 1) |
                I2C_CR2_RD_WRN |
                (length << 16) |
                I2C_CR2_START |
                I2C_CR2_AUTOEND;

    for (int i = 0; i < length; i++)
    {
        while (!(I2C1->ISR & I2C_ISR_RXNE));
        buffer[i] = I2C1->RXDR;
    }

    while (!(I2C1->ISR & I2C_ISR_STOPF));

    I2C1->ICR |= I2C_ICR_STOPCF;
}


void compassInit(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB->MODER &= ~(0xFUL << 12);
    GPIOB->MODER |=  (0xAUL << 12);

    GPIOB->AFR[0] |= (4 << 24) | (4 << 28);

    GPIOB->OTYPER |= (1 << 6) | (1 << 7);

    I2C1->CR1 &= ~I2C_CR1_PE;

    I2C1->TIMINGR = 0x2000090E;

    I2C1->CR1 |= I2C_CR1_PE;

    i2cWriteRegister(0x00, 0x14);
    i2cWriteRegister(0x01, 0x20);
    i2cWriteRegister(0x02, 0x00);
}


void compassRead(CompassData* data)
{
    uint8_t raw[6];

    i2cReadRegisters(0x03, raw, 6);

    data->x = (raw[0] << 8) | raw[1];
    data->z = (raw[2] << 8) | raw[3];
    data->y = (raw[4] << 8) | raw[5];

    data->heading = atan2(data->y, data->x) * 180 / 3.14159;

    if (data->heading < 0)
    {
        data->heading += 360;
    }

    data->timestamp = systemMillis;
}
