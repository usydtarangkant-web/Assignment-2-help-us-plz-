#ifndef COMPASS_H                 // Start include guard to prevent multiple inclusion
#define COMPASS_H

#include "stm32f303xc.h"         // Include STM32 register definitions and data types


/* Structure used to store compass data */
typedef struct
{
    int16_t x;                   // Raw magnetometer X-axis value
    int16_t y;                   // Raw magnetometer Y-axis value
    int16_t z;                   // Raw magnetometer Z-axis value

    float heading;              // Calculated compass heading in degrees

    uint32_t timestamp;         // Time in milliseconds when data was read

} CompassData;


/* Initialise compass sensor and I2C interface */
void compassInit(void);


/* Read compass values and store them in structure */
int compassRead(CompassData* data);


#endif
