/*=========================================================
accel.h
=========================================================*/
#ifndef ACCEL_H
#define ACCEL_H

#include "stm32f303xc.h"

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;

    float ax;
    float ay;
    float az;

} AccelData;

void accelInit(void);
int accelRead(AccelData *data);

#endif
