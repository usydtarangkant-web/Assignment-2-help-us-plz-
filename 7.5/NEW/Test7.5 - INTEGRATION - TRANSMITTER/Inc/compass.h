#ifndef COMPASS_H
#define COMPASS_H

#include "stm32f303xc.h"

/* Compass values read from sensor */
typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
    float heading;
    uint32_t timestamp;
} CompassData;

/* Set up the compass module */
void compassInit(void);

/* Read one set of compass values */
int compassRead(CompassData *data);

#endif
