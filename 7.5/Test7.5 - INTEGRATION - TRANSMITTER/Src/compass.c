#include "compass.h"
#include <math.h>

#define COMPASS_ADDR 0x3C

#define CFG_REG_A_M   0x60
#define CFG_REG_B_M   0x61
#define CFG_REG_C_M   0x62
#define OUTX_L_REG_M  0x68

#define X_OFFSET   (-370.0f)
#define Y_OFFSET   (40.0f)

extern volatile uint32_t tick_ms;

static int waitSet(uint32_t flag)
{
    volatile uint32_t i;

    for (i = 0; i < 100000; i++)
    {
        if (I2C1->ISR & flag)
        {
            return 0;
        }
    }

    return -1;
}

static void i2cInit(void)
{
    RCC->AHBENR  |= RCC_AHBENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB->MODER &= ~(0xFUL << 12);
    GPIOB->MODER |=  (0xAUL << 12);

    GPIOB->OTYPER |= (1U << 6) | (1U << 7);

    GPIOB->PUPDR &= ~(0xFUL << 12);
    GPIOB->PUPDR |=  (0x5UL << 12);

    GPIOB->AFR[0] &= ~(0xFFUL << 24);
    GPIOB->AFR[0] |=  (0x44UL << 24);

    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->TIMINGR = 0x2000090E;
    I2C1->CR1 |= I2C_CR1_PE;
}

static int i2cWriteReg(uint8_t reg, uint8_t val)
{
    I2C1->ICR |= I2C_ICR_STOPCF;

    I2C1->CR2 =
        COMPASS_ADDR |
        (2U << I2C_CR2_NBYTES_Pos) |
        I2C_CR2_START;

    if (waitSet(I2C_ISR_TXIS)) return -1;
    I2C1->TXDR = reg;

    if (waitSet(I2C_ISR_TXIS)) return -1;
    I2C1->TXDR = val;

    if (waitSet(I2C_ISR_TC)) return -1;

    I2C1->CR2 |= I2C_CR2_STOP;

    return 0;
}

static int i2cReadBurst(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;

    I2C1->CR2 =
        COMPASS_ADDR |
        (1U << I2C_CR2_NBYTES_Pos) |
        I2C_CR2_START;

    if (waitSet(I2C_ISR_TXIS)) return -1;
    I2C1->TXDR = reg;

    if (waitSet(I2C_ISR_TC)) return -1;

    I2C1->CR2 =
        COMPASS_ADDR |
        I2C_CR2_RD_WRN |
        ((uint32_t)len << I2C_CR2_NBYTES_Pos) |
        I2C_CR2_START |
        I2C_CR2_AUTOEND;

    for (i = 0; i < len; i++)
    {
        if (waitSet(I2C_ISR_RXNE)) return -1;
        buf[i] = I2C1->RXDR;
    }

    return 0;
}

void compassInit(void)
{
    i2cInit();

    i2cWriteReg(CFG_REG_A_M, 0x8C);
    i2cWriteReg(CFG_REG_B_M, 0x01);
    i2cWriteReg(CFG_REG_C_M, 0x10);
}

int compassRead(CompassData *data)
{
    uint8_t raw[6];
    float x_corrected;
    float y_corrected;

    if (i2cReadBurst(OUTX_L_REG_M | 0x80, raw, 6))
    {
        return -1;
    }

    data->x = (int16_t)((raw[1] << 8) | raw[0]);
    data->y = (int16_t)((raw[3] << 8) | raw[2]);
    data->z = (int16_t)((raw[5] << 8) | raw[4]);

    x_corrected = (float)data->x - X_OFFSET;
    y_corrected = (float)data->y - Y_OFFSET;

    data->heading = atan2f(y_corrected, x_corrected) * (180.0f / 3.14159265f);

    if (data->heading < 0.0f)
    {
        data->heading += 360.0f;
    }

    data->timestamp = tick_ms;

    return 0;
}
