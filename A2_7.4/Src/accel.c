/*=========================================================

Dedicated accelerometer I2C functions
Does NOT reuse compass address functions

LSM303AGR / STM32F3 Discovery style
=========================================================*/
#include "accel.h"

/* Accelerometer address:
7-bit = 0x19
8-bit shifted = 0x32
*/
#define ACCEL_ADDR 0x32

/* Registers */
#define CTRL_REG1_A   0x20
#define CTRL_REG4_A   0x23
#define OUT_X_L_A     0x28

/*-------------------------------------------------------
wait for I2C flag
-------------------------------------------------------*/
static int waitSet(uint32_t flag)
{
    volatile uint32_t i;

    for (i = 0; i < 100000; i++)
    {
        if (I2C1->ISR & flag)
            return 0;
    }

    return -1;
}

/*-------------------------------------------------------
write one register to accelerometer
-------------------------------------------------------*/
static int accelWriteReg(uint8_t reg, uint8_t val)
{
    I2C1->ICR |= I2C_ICR_STOPCF;

    I2C1->CR2 =
        ACCEL_ADDR |
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

/*-------------------------------------------------------
read multiple bytes
-------------------------------------------------------*/
static int accelReadBurst(uint8_t reg,
                          uint8_t *buf,
                          uint8_t len)
{
    uint8_t i;

    /* send register first */
    I2C1->CR2 =
        ACCEL_ADDR |
        (1U << I2C_CR2_NBYTES_Pos) |
        I2C_CR2_START;

    if (waitSet(I2C_ISR_TXIS)) return -1;
    I2C1->TXDR = reg;

    if (waitSet(I2C_ISR_TC)) return -1;

    /* restart read mode */
    I2C1->CR2 =
        ACCEL_ADDR |
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

/*-------------------------------------------------------
initialise accelerometer
-------------------------------------------------------*/
void accelInit(void)
{
    /* 100 Hz data rate, XYZ enable */
    accelWriteReg(CTRL_REG1_A, 0x57);

    /* high resolution, +/-2g */
    accelWriteReg(CTRL_REG4_A, 0x08);
}

/*-------------------------------------------------------
read accelerometer
-------------------------------------------------------*/
int accelRead(AccelData *data)
{
    uint8_t raw[6];

    if (accelReadBurst(OUT_X_L_A | 0x80, raw, 6))
        return -1;

    data->x = (int16_t)((raw[1] << 8) | raw[0]);
    data->y = (int16_t)((raw[3] << 8) | raw[2]);
    data->z = (int16_t)((raw[5] << 8) | raw[4]);

    /* 12-bit left aligned */
    data->x >>= 4;
    data->y >>= 4;
    data->z >>= 4;

    /* approximate g values */
    data->ax = data->x * 0.001f;
    data->ay = data->y * 0.001f;
    data->az = data->z * 0.001f;

    return 0;
}
