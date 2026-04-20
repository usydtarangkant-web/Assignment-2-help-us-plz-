/*
------------------------------------------------------------
compass.c

STM32F3 Discovery Board Compass Module

Uses onboard LSM303AGR magnetometer via I2C1.

Functions:
1. Initialise I2C interface
2. Configure compass sensor
3. Read raw X, Y, Z magnetic values
4. Apply simple offset calibration
5. Calculate heading angle (0 to 360 deg)
6. Store timestamp
------------------------------------------------------------
*/

#include "compass.h"
#include <math.h>


/*----------------------------------------------------------
I2C slave address of magnetometer
----------------------------------------------------------*/
#define COMPASS_ADDR 0x3C


/*----------------------------------------------------------
Magnetometer register addresses
----------------------------------------------------------*/
#define CFG_REG_A_M   0x60     /* Configuration register A */
#define CFG_REG_B_M   0x61     /* Configuration register B */
#define CFG_REG_C_M   0x62     /* Configuration register C */
#define OUTX_L_REG_M  0x68     /* First output register containing X low byte */


/*----------------------------------------------------------
Calibration offsets found experimentally by rotating board
flat on table.

X had range: -190 to -550
Y had range: 210 to -130

Offset found by (min+max)/2

Used to center X/Y values around zero before atan2().
----------------------------------------------------------*/
#define X_OFFSET   (-370.0f)
#define Y_OFFSET   (40.0f)


/*----------------------------------------------------------
Global millisecond timer declared in main.c
----------------------------------------------------------*/
extern volatile uint32_t tick_ms;


/*
------------------------------------------------------------
Wait until selected I2C flag becomes set

Returns:
0  = success
-1 = timeout
------------------------------------------------------------
*/
static int waitSet(uint32_t flag)
{
    volatile uint32_t i;

    for (i = 0; i < 100000; i++)
    {
        if (I2C1->ISR & flag) // Access the ISR register in the I2C1 peripheral, bitwise & with flag bitmasks to check if equal
        {
            return 0;
        }
    }

    return -1;
}


/*
------------------------------------------------------------
Initialise I2C1 peripheral

Pins:
PB6 = SCL
PB7 = SDA
------------------------------------------------------------
*/
static void i2cInit(void)
{
    /* Enable clocks */
    RCC->AHBENR  |= RCC_AHBENR_GPIOBEN; // set the correct bit in AHBENR register in RCC peripheral to be enabled using OR
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; // same as above

    /* PB6/PB7 alternate function mode */
    GPIOB->MODER &= ~(0xFUL << 12); // clears bits 12:15 (where PB6 and PB7 are)
    GPIOB->MODER |=  (0xAUL << 12); // places hex A (0101) into bits 12:15 and sets as alternate function mode (<< 12 shifts left by 12)

    /* Open-drain outputs */
    GPIOB->OTYPER |= (1U << 6) | (1U << 7); // moves bit 1 to positions of pin 6 and 7 (makes them open-drain)

    /* Pull-up resistors */
    GPIOB->PUPDR &= ~(0xFUL << 12);
    GPIOB->PUPDR |=  (0x5UL << 12);

    /* Alternate Function 4 maps to I2C1 */
    GPIOB->AFR[0] &= ~(0xFFUL << 24); // set PB6 and PB7 to alternate function number 4, which connects pins internally to the I2C1 peripheral
    GPIOB->AFR[0] |=  (0x44UL << 24);

    /* Disable before timing setup to avoid unintended operation*/
    I2C1->CR1 &= ~I2C_CR1_PE;

    /* 100 kHz timing (8 MHz clock) */
    I2C1->TIMINGR = 0x2000090E;

    /* Enable I2C */
    I2C1->CR1 |= I2C_CR1_PE;
}


/*
------------------------------------------------------------
Write one byte to one compass register

Returns:
0  = success
-1 = fail
------------------------------------------------------------
*/
static int i2cWriteReg(uint8_t reg, uint8_t val) // This function tells compass sensor to store val (byte 2) in reg (byte 1)
{
    /* Clear previous STOP flag */
    I2C1->ICR |= I2C_ICR_STOPCF;

    /* Send 2 bytes: register + value to start write transfer */
    I2C1->CR2 =
        COMPASS_ADDR |
        (2U << I2C_CR2_NBYTES_Pos) |
        I2C_CR2_START;

    if (waitSet(I2C_ISR_TXIS)) return -1;
    I2C1->TXDR = reg;

    if (waitSet(I2C_ISR_TXIS)) return -1;
    I2C1->TXDR = val;

    if (waitSet(I2C_ISR_TC)) return -1;

    /* Stop condition */
    I2C1->CR2 |= I2C_CR2_STOP;

    return 0;
}


/*
------------------------------------------------------------
Read multiple sequential registers

Inputs:
reg = first register address
buf = destination buffer
len = number of bytes

Returns:
0  = success
-1 = fail
------------------------------------------------------------
*/
static int i2cReadBurst(uint8_t reg,
                        uint8_t *buf,
                        uint8_t len)
{
    uint8_t i;

    /* Send register address first */
    I2C1->CR2 =
        COMPASS_ADDR |
        (1U << I2C_CR2_NBYTES_Pos) |
        I2C_CR2_START;

    if (waitSet(I2C_ISR_TXIS)) return -1;
    I2C1->TXDR = reg;

    if (waitSet(I2C_ISR_TC)) return -1;

    /* Restart in read mode (Repeated start)
     After the repeated start condition, use the peripheral’s I2C address again, but with a read bit
    */
    I2C1->CR2 =
        COMPASS_ADDR |
        I2C_CR2_RD_WRN |
        ((uint32_t)len << I2C_CR2_NBYTES_Pos) |
        I2C_CR2_START |
        I2C_CR2_AUTOEND; // STM32 automatically sends STOP after final byte.

    for (i = 0; i < len; i++)
    {
        if (waitSet(I2C_ISR_RXNE)) return -1;

        buf[i] = I2C1->RXDR;
    }

    return 0;
}

/*

If called like:

i2cReadBurst(0x68, raw, 6);

Then bus sequence is:

START
Address + Write
0x68          (register address)

RESTART
Address + Read

Read 6 bytes:
B1 B2 B3 B4 B5 B6

STOP
 */


/*
------------------------------------------------------------
Initialise compass sensor
------------------------------------------------------------
*/
void compassInit(void)
{
    /* Initialise I2C hardware first */
    i2cInit();

    /*
    Configure sensor:
    Continuous conversion mode
    Output data rate enabled
    */
    i2cWriteReg(CFG_REG_A_M, 0x8C); // Write 0x8C into register 0x60
    i2cWriteReg(CFG_REG_B_M, 0x01); // Write 0x01 into register 0x61
    i2cWriteReg(CFG_REG_C_M, 0x10); // Write 0x10 into register 0x62
}


/*
------------------------------------------------------------
Read compass values

Outputs:
data->x
data->y
data->z
data->heading
data->timestamp

Returns:
0  = success
-1 = fail
------------------------------------------------------------
*/
int compassRead(CompassData *data)
{
    uint8_t raw[6];

    float xCal;
    float yCal;

    /* Read 6 bytes starting from X register */
    if (i2cReadBurst(OUTX_L_REG_M | 0x80, raw, 6))
    {
        return -1;
    }

    /*
    Convert bytes to signed 16-bit values

    Sensor order observed on board:
    X, Y, Z
    */
    data->x = (int16_t)((raw[1] << 8) | raw[0]);
    data->y = (int16_t)((raw[3] << 8) | raw[2]);
    data->z = (int16_t)((raw[5] << 8) | raw[4]);

    /*
    Apply offset calibration
    This recentres raw values around zero.
    */
    xCal = (float)data->x - X_OFFSET;
    yCal = (float)data->y - Y_OFFSET;

    /*
    Calculate heading angle in degrees
    atan2 gives result from -180 to +180
    */
    data->heading =
        atan2(yCal, xCal) *
        180.0f / 3.14159f;

    /* Correct board alignment */
    data->heading += 45.0f;

    /* Convert to 0 to 360 degrees */
    if (data->heading < 0.0f)
    {
        data->heading += 360.0f;
    }

    if (data->heading >= 360)
    {
        data->heading -= 360.0f;
    }

    /* Save current timestamp */
    data->timestamp = tick_ms;

    return 0;
}
