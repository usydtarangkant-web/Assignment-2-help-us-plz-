//timer.h (for parts a, b, d in task 7.2.2)

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "stm32f303xc.h"

typedef void (*Timer_Callback_t)(void);

typedef struct {
    uint8_t slot;          //Private timer slot index
    uint8_t allocated;     //1 = slot allocated, 0 = not allocated
} Timer_t;
//state functions
//Part a
void Timer_Init(Timer_t *tim, uint32_t period_ms, Timer_Callback_t cb);
void Timer_Tick(Timer_t *tim);
void Timer_Start(Timer_t *tim);
void Timer_Stop(Timer_t *tim);

//Part b
uint32_t Timer_GetPeriod(const Timer_t *tim);
void Timer_SetPeriod(Timer_t *tim, uint32_t period_ms);

//Part d
void Timer_OnceMs(Timer_t *tim, uint32_t delay_ms, Timer_Callback_t cb);

//For 100 us servo timing
void Timer_InitTicks(Timer_t *tim, uint32_t period_ticks, Timer_Callback_t cb);
void Timer_OnceTicks(Timer_t *tim, uint32_t delay_ticks, Timer_Callback_t cb);

#endif
