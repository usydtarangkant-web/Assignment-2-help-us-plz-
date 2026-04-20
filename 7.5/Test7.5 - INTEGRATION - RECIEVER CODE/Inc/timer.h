#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

typedef void (*Timer_Callback_t)(void);

typedef struct
{
    uint8_t slot;
    uint8_t allocated;
} Timer_t;

void Timer_Init(Timer_t *tim, uint32_t period_ms, Timer_Callback_t cb);
void Timer_InitTicks(Timer_t *tim, uint32_t period_ticks, Timer_Callback_t cb);
void Timer_Tick(Timer_t *tim);
void Timer_Start(Timer_t *tim);
void Timer_Stop(Timer_t *tim);
uint32_t Timer_GetPeriod(const Timer_t *tim);
void Timer_SetPeriod(Timer_t *tim, uint32_t period_ms);
void Timer_OnceMs(Timer_t *tim, uint32_t delay_ms, Timer_Callback_t cb);
void Timer_OnceTicks(Timer_t *tim, uint32_t delay_ticks, Timer_Callback_t cb);

#endif
