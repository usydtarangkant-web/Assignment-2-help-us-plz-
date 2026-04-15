#ifndef COMPASS_H
#define COMPASS_H

#include "stm32f303xc.h"

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;

    float heading;

    uint32_t timestamp;

} CompassData;

void compassInit(void);
void compassRead(CompassData* data);

#endif
