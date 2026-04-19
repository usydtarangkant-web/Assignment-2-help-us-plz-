#include "compass.h"            // Include compass header file
#include <math.h>               // Include math functions such as atan2()


/* I2C address of onboard magnetometer (LSM303AGR) */
#define COMPASS_ADDR 0x3C


/* Magnetometer register addresses */
#define CFG_REG_A_M   0x60      // Configuration register A
#define CFG_REG_B_M   0x61      // Configuration register B
#define CFG_REG_C_M   0x62      // Configuration register C
#define OUTX_L_REG_M  0x68      // First output register for X-axis data


/* Millisecond timer variable declared in main.c */
extern volatile uint32_t tick_ms;


/* Wait until selected I2C status flag becomes set */
int waitSet(uint32_t flag)
{
    for (volatile int i = 0; i < 100000; i++)   // Simple timeout loop
    {
        if (I2C1->ISR & flag)                   // Check if flag is set
        {
            return 0;                          // Success
        }
    }

    return -1;                                 // Timeout error
}


/* Initialise I2C1 peripheral and GPIO pins */
void i2cInit(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;         // Enable GPIOB clock
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;        // Enable I2C1 clock

    /* Set PB6 and PB7 to alternate function mode */
    GPIOB->MODER &= ~(0xFUL << 12);
    GPIOB->MODER |=  (0xAUL << 12);

    /* Set PB6 and PB7 as open-drain outputs */
    GPIOB->OTYPER |= (1 << 6) | (1 << 7);

    /* Enable pull-up resistors on PB6 and PB7 */
    GPIOB->PUPDR &= ~(0xFUL << 12);
    GPIOB->PUPDR |=  (0x5UL << 12);

    /* Select Alternate Function 4 for I2C */
    GPIOB->AFR[0] &= ~(0xFFUL << 24);
    GPIOB->AFR[0] |=  (0x44UL << 24);

    I2C1->CR1 &= ~I2C_CR1_PE;                 // Disable I2C before setup

    I2C1->TIMINGR = 0x2000090E;              // Set timing for 100kHz I2C

    I2C1->CR1 |= I2C_CR1_PE;                 // Enable I2C
}


/* Write one byte to one sensor register */
int i2cWriteReg(uint8_t addr, uint8_t reg, uint8_t val)
{
    I2C1->ICR |= I2C_ICR_STOPCF;             // Clear STOP flag

    /* Configure write transfer */
    I2C1->CR2 =
        addr |                               // Slave address
        (2 << I2C_CR2_NBYTES_Pos) |          // Send 2 bytes total
        I2C_CR2_START;                       // Generate START condition

    if (waitSet(I2C_ISR_TXIS)) return -1;   // Wait until TX ready
    I2C1->TXDR = reg;                       // Send register address

    if (waitSet(I2C_ISR_TXIS)) return -1;   // Wait until TX ready
    I2C1->TXDR = val;                       // Send register data

    if (waitSet(I2C_ISR_TC)) return -1;     // Wait for transfer complete

    I2C1->CR2 |= I2C_CR2_STOP;              // Generate STOP condition

    return 0;                               // Success
}


/* Read multiple bytes starting from one register */
int i2cReadBurst(uint8_t addr, uint8_t reg, uint8_t* buf, uint8_t len)
{
    I2C1->ICR |= I2C_ICR_STOPCF;            // Clear STOP flag

    /* Send register address first */
    I2C1->CR2 =
        addr |
        (1 << I2C_CR2_NBYTES_Pos) |
        I2C_CR2_START;

    if (waitSet(I2C_ISR_TXIS)) return -1;   // Wait for TX ready
    I2C1->TXDR = reg;                       // Send start register

    if (waitSet(I2C_ISR_TC)) return -1;     // Wait for complete

    /* Restart transaction in read mode */
    I2C1->CR2 =
        addr |
        I2C_CR2_RD_WRN |                    // Read mode
        (len << I2C_CR2_NBYTES_Pos) |       // Number of bytes to receive
        I2C_CR2_START |                     // Restart
        I2C_CR2_AUTOEND;                   // Auto STOP at end

    for (uint8_t i = 0; i < len; i++)
    {
        if (waitSet(I2C_ISR_RXNE)) return -1; // Wait until byte received

        buf[i] = I2C1->RXDR;                  // Store received byte
    }

    return 0;                                 // Success
}


/* Initialise magnetometer sensor */
void compassInit(void)
{
    i2cInit();                               // Initialise I2C first

    i2cWriteReg(COMPASS_ADDR, CFG_REG_A_M, 0x8C); // Continuous mode / ODR
    i2cWriteReg(COMPASS_ADDR, CFG_REG_B_M, 0x01); // Gain setting
    i2cWriteReg(COMPASS_ADDR, CFG_REG_C_M, 0x10); // Data ready config
}


/* Read magnetometer values and calculate heading */
int compassRead(CompassData* data)
{
    uint8_t raw[6];                          // Temporary raw byte array

    /* Read 6 bytes from output registers */
    if (i2cReadBurst(COMPASS_ADDR, OUTX_L_REG_M | 0x80, raw, 6))
    {
        return -1;                          // Return error if failed
    }

    /* Combine low and high bytes into signed 16-bit values */
    data->x = (raw[1] << 8) | raw[0];
    data->y = (raw[3] << 8) | raw[2];
    data->z = (raw[5] << 8) | raw[4];

    /* Calculate heading angle in degrees */
    data->heading =
        atan2((float)data->y, (float)data->x) *
        180.0f / 3.14159f;

    /* Convert negative angle into 0–360 range */
    if (data->heading < 0)
    {
        data->heading += 360.0f;
    }

    data->timestamp = tick_ms;              // Save current time

    return 0;                               // Success
}
